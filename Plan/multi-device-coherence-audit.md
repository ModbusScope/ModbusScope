# Multi-Device Coherence Audit

## Summary

Device → adapter routing works where it was deliberately built for multiple adapters. It breaks down at every entry point that predates the adapter split, and the model layer has no way to tell the rest of the app when the device list changes.

| Question | Verdict |
|---|---|
| Is it adapter-dependent? | **Partially.** The routing spine (grouping, sessions, result merge, register creation) is fully adapter-aware. Four entry points still assume a single Modbus adapter and device 1. |
| Is it coherent everywhere? | **Mostly.** Ownership is now resolved by two rules instead of three (adapter reconciliation, and project-file load), and the settings dialog no longer mutates the model before OK. Pressing OK still marks every adapter as explicitly configured, even ones with no tabs. |
| Is the work finished? | **Closer.** `deviceListChanged()` now has consumers, the settings dialog is transactional, and `ScopeController::start()` now validates every referenced device before logging begins. An unknown device still resolves to `"modbus"` everywhere else, and the project file still writes device→adapter twice. |

Scope: `src/models`, `src/communication`, `src/ProtocolAdapter`, `src/dialogs`, `src/importexport`. Base: `f019fc9` on `claude/reconcile-devices-review-k18elr`. Analysis only — no code changed. Severity reflects likelihood of silent wrong behaviour, not implementation cost.

---

## 1. How multi-device is meant to work

Three facts define the whole design; every finding below traces back to one of them.

**A device ID is a global key.** `SettingsModel::_devices` is a `QMap<deviceId_t, Device>`, and each `Device` carries only a name and an owning `adapterId` (`src/models/device.h:8`). Every adapter has its own device-ID namespace, but the app flattens them into one. Two adapters cannot both own a device numbered 1.

**The device travels inside the expression string.** `ExpressionParser` matches `${address@deviceId:type}` and stores the whole matched text as the `DataPoint` address, alongside the parsed device ID (`src/datahandling/expressionparser.cpp:97`). The app never strips the `@N`; it forwards the raw expression and lets the adapter resolve the device itself. An omitted `@N` silently means device 1.

**Polling fans out and merges back.** `AdapterPoll::buildAdapterGroups()` maps each register through `adapterIdForDevice()` into a per-adapter group that remembers its original indices; one session is started per group; results are merged back into register order once every group has answered.

```
GraphDataModel → ExpressionParser → buildAdapterGroups (adapterIdForDevice)
                                            │
                              ┌─────────────┴─────────────┐
                       session: modbus              session: dummy
                       devices 1, 3                 device 2
                              │                             │
                              └─────────── merge (by index) ┘
                                   (all groups must answer)
```

Ownership itself is decided by `reconcileDevicesWithAdapters()`, which walks adapters with a stored config first, then unconfigured ones (with `"modbus"` pulled to the front), and lets the first adapter to name a device ID claim it. That method is sound and well documented. The problems are around it, not in it.

---

## 2. What is genuinely adapter-dependent

These paths were built for the multi-adapter world and hold up. They are the reference for what the rest should look like.

- **Add-register flow** — device first, then adapter, then that adapter's `dataPointSchema`, then `buildExpression(fields, type, deviceId)`. Disables Add when the device's adapter has no live manager.
- **Expression describe** — `managerForDescribeRow()` resolves each row's device to its own adapter, and marks rows for missing devices as *Invalid device* instead of guessing.
- **Device config tabs** — each tab carries its own adapter combo and rebuilds the schema form when the adapter changes, seeding from that adapter's defaults.
- **Settings pages** — one page per adapter per schema key, labelled `Connections [modbus]`. Reads and writes are scoped to a single adapter's config.
- **Poll fan-out and merge** — per-adapter groups, per-adapter sessions, index-preserving merge, and a guard that skips phantom adapters so the cycle can't hang on them.
- **Per-adapter wire config** — `configForWire()` applies each adapter's own `devices.maxItems`, and `effectiveConfig()` backfills newly added top-level keys from defaults.
- **Project file round-trip** — devices persist `adapter.type`; adapters present in the file but not in the model are preserved rather than dropped.
- **Ownership reconciliation** — re-runs on every describe, survives reconnects, and prefers explicitly configured adapters over ones sitting on untouched defaults.

---

## 3. Entry points that still assume one adapter

### F1 — An unknown device silently routes to `"modbus"` [High]

`Device`'s constructor hardcodes `_adapterId("modbus")`, and `adapterIdForDevice()` returns a default-constructed `Device`'s adapter ID for any device the model doesn't know. There is no "unassigned" or "unknown" state.

The consequence sits in `buildAdapterGroups()`: a register referencing a deleted or never-created device produces a group keyed `"modbus"`, and `${40001@7}` is sent to the Modbus adapter, which has no device 7. The failure surfaces as adapter-side errors or invalid values, never as "this register points at a device that doesn't exist".

**Partially addressed:** `ScopeController::start()` — `buildAdapterGroups()`'s only caller — now checks `hasDevice()` for every referenced device before starting a session, and refuses with a message naming the device instead of misrouting it. The underlying fallback is untouched, though, and still misroutes silently everywhere else `adapterIdForDevice()` is called outside session start (`AdapterDeviceSettings`, `AddRegisterWidget`, `RegisterDialog`, `ExpressionsDialog`) — this is the gap "Where to start" item 2 (an explicit "unassigned" adapter ID) still closes.

Where: `src/models/device.cpp:6` · `src/models/settingsmodel.cpp:141` · `src/communication/adapterpoll.cpp:218` · `src/controllers/scopecontroller.cpp:181`

### F2 — MBC import is hardcoded to device 1 and gated on the wrong adapter [High]

`MbcRegisterData::toExpression()` emits `${address}` or `${address:type}` — never an `@deviceId`. Every imported register therefore lands on device 1, whichever adapter happens to own it.

The gate is equally loose: `isMbcCompatible()` returns true if *any* registered adapter reports the capability, so the import is enabled even when device 1 belongs to an adapter that does not support MBC. There is no device picker in the import flow at all.

Where: `src/MbcInterface/mbcregisterdata.cpp:181` · `src/models/settingsmodel.cpp:220` · `src/controllers/scopecontroller.cpp:237`

### F4 — Adapter diagnostics are indistinguishable between adapters [Medium]

`AdapterManager::onAdapterDiagnostic()` forwards the message to the shared `scope.comm.adapter` category with no adapter ID attached, even though the manager knows its own `_adapterId`. `AdapterClient`'s own warnings do include it.

With two adapters running, the diagnostic log interleaves both streams with no way to tell them apart — precisely when you need it most.

Where: `src/ProtocolAdapter/adaptermanager.cpp:146`

### F5 — Communication statistics are aggregate only [Low]

Success and error counts are accumulated per register across all adapters, and median poll time is derived from graph 0's series. Nothing is broken by this, but a user with two adapters cannot see which one is failing or which one is slowing the cycle down.

Where: `src/communication/communicationstats.cpp:22`

---

## 4. Coherence breaks across layers

**Root cause:** Device ownership is decided in two places with two different rules: `reconcileDevicesWithAdapters()` (stored-config priority, run whenever the Devices page opens or an adapter describes) and `ProjectFileHandler::applyDeviceSettings()` (file wins, fall back to `"modbus"`, run on project load). Each is individually defensible; together they can still disagree about "who owns device N?" depending on which one ran last.

### F8 — Pressing OK marks every adapter as explicitly configured [Medium]

`AdapterDeviceSettings::acceptValues()` loops over *all* valid adapters and calls `setAdapterCurrentConfig()` for each, writing an empty `devices` array for adapters with no tabs. That setter unconditionally sets `hasStoredConfig = true`.

`hasStoredConfig` is the load-bearing input to reconciliation's priority ordering and to `buildCurrentAdapters()`'s export decision. After one trip through Settings, the flag no longer means "the user configured this adapter" — it means "the user once opened Settings" — and the `"modbus"`-first tie-break, which only applies within the unconfigured group, is permanently disabled.

Where: `src/dialogs/adapterdevicesettings.cpp:332` · `src/models/settingsmodel.cpp:244` · `src/importexport/projectfilehandler.cpp:170`

### F9 — The device limit warns, but doesn't prevent adding a device past it [Low]

Exceeding an adapter's device limit produces an orange label and nothing else — the tab is created, the device enters the model, and the config is stored. At session start `configForWire()` still drops devices from the *end* of the array once it exceeds `maxDevices()` (the smaller of the schema's `maxItems` and the adapter's license-aware `capabilities.maxDevices`).

**Addressed:** those devices used to stay in `SettingsModel` with `buildAdapterGroups()` silently routing their registers to an adapter that was never told they exist. `ScopeController::sessionValidationError()` now checks, for every referenced device, that it exists, survived `configForWire()`'s truncation, and is owned by a live adapter — `start()` refuses with a message naming the device instead of starting a session that would misroute it. Covered directly by `tst_scopecontroller.cpp`'s `startWithUnknownDeviceEmitsError`, `startWithDeviceTruncatedByAdapterLimitEmitsError` and `startWithDeviceOwnedByUnavailableAdapterEmitsError` (the old `tst_dummydevicelimit` integration test still only passes an empty register list, so it exercises the adapter-side truncation but not this gate). What remains: the orange label is still advisory only — nothing stops a device being added past the limit in the first place, it just now fails cleanly at Start instead of silently at runtime.

Where: `src/models/adapterdata.cpp:200` · `src/dialogs/adapterdevicesettings.cpp:229` · `src/controllers/scopecontroller.cpp:120` · `:181` · `tests/controllers/tst_scopecontroller.cpp:63` · `tests/integration/tst_dummydevicelimit.cpp:68`

### F11 — The project file stores device→adapter twice and reads one [Low]

Each exported device carries both a numeric `adapterId` index into the adapters array and an `adapter.type` string. The parser reads and validates both, but `applyDeviceSettings()` uses only `adapterType`, falling back to `"modbus"` when it is empty. The index is write-only and can disagree with the string after any hand edit.

Where: `src/importexport/projectfilejsonexporter.cpp:97` · `src/importexport/projectfilejsonparser.cpp:212` · `src/importexport/projectfilehandler.cpp:304`

### F12 — A device's `connectionId` is never validated against the connections list [Low]

`connectionId` is rendered as a plain integer field from the schema, with no awareness that it references another array in the same config. Deleting a connection on the Connections page leaves every device that pointed at it dangling, with no warning on either page. This is the main linking field in a multi-device Modbus setup.

Where: `src/dialogs/adaptersettings.cpp:158` · `src/customwidgets/deviceconfigtab.cpp:110`

---

## 5. Runtime behaviour with more than one adapter

### F14 — One hung adapter stalls the whole poll cycle, with no timeout [High]

`onReadDataResult()` withholds `registerDataReady` until `_pendingResultAdapters` is empty, and the poll timer is only restarted after the merge. `AdapterClient` has a handshake timeout but no `readData` timeout — the only guards are process death and framing errors.

An adapter that accepts the request and never answers therefore freezes logging indefinitely: no data, no error, no timer. With one adapter this was already possible; fanning out to N adapters makes the whole session only as reliable as its slowest member.

Where: `src/communication/adapterpoll.cpp:132` · `src/ProtocolAdapter/adapterclient.cpp:95`

### F15 — Any single adapter's failure terminates the entire session [Medium]

`AdapterHub::onManagerSessionError()` re-emits the error unqualified, and `AdapterPoll::onSessionError()` drops straight to `Inactive`. Losing one adapter stops logging for all of them, and the message reaching the user names neither the adapter nor the affected devices.

This may well be the intended policy, but it is worth making explicit — the alternative (continue on the surviving adapters, mark the lost ones' registers invalid) is a plausible design and nothing in the code records which was chosen.

Where: `src/ProtocolAdapter/adapterhub.cpp:211` · `src/communication/adapterpoll.cpp:197`

### F16 — A result-count mismatch drops a whole group into default values [Low]

When an adapter returns a different number of results than it was given expressions, the merge logs a warning and skips the group. Those slots keep default-constructed `ResultDouble`s and flow onward as if they were real. A misbehaving adapter degrades its own registers quietly rather than failing the cycle.

Where: `src/communication/adapterpoll.cpp:153`

---

## 6. Test coverage

Reconciliation and dialog cancel semantics now have direct, model-level coverage. The remaining gaps sit where the routing actually runs.

| Area | Coverage | Gap |
|---|---|---|
| `reconcileDevices…` | Direct | `tst_settingsmodel` now covers it at the model level (single-emission, never-prune, name preservation across reassignment, no-op silence). |
| AdapterPoll grouping | Single adapter | Every assertion indexes `_startCalls[0]`. Nothing exercises two groups, the index-preserving merge, partial arrival ordering, or F16's mismatch path. |
| Mixed adapter states | Direct | `tst_adapterhub::initAdapterReinitializesOnlyIdleManagers` now covers the former F13 deadlock scenario directly. |
| Device limit | Direct | `tst_scopecontroller` now covers a register aimed at an unknown, truncated, or unavailable-adapter device directly, at the layer that gates Start. `tst_dummydevicelimit` still only confirms the adapter-side session starts after truncation, using an empty register list. |
| Cancel semantics | Direct | `cancelDiscardsDeviceListEdits`, `cancelDiscardsDeviceFieldEdits`, `nameChangeDoesNotReachModelUntilAccept` and `adapterChangeDoesNotReachModelUntilAccept` now assert renames and reassignments are discarded on cancel. |

---

## 7. Where to start

Ordered by how much they unblock, not by size.

1. **Add a read-data timeout (F14).** Per-adapter, reusing the handshake-timeout machinery. Decide alongside it whether a timed-out adapter fails the session (current F15 policy) or degrades to invalid results for its own registers.

2. **Introduce an explicit "unassigned" adapter ID (F1).** Stop letting `"modbus"` double as the default. Every silent misroute above depends on an unknown device resolving to a real adapter — `ScopeController::start()` now catches it at session start, but `AdapterDeviceSettings`, `AddRegisterWidget`, `RegisterDialog` and `ExpressionsDialog` all still call `adapterIdForDevice()` directly.

3. **Give MBC import a device (F2).** Add a device selector, emit `@deviceId` from `toExpression()`, and gate the action on the selected device's adapter rather than on any adapter.

4. **Stop `acceptValues()` flagging untouched adapters (F8).** Write only adapters that actually own tabs, so `hasStoredConfig` keeps meaning what reconciliation and the exporter assume it means.

5. **Add a two-adapter `AdapterPoll` test.** Reconciliation now has direct model-level coverage via `tst_settingsmodel`; the fan-out/merge remains the one piece of multi-device machinery with no multi-adapter test.
