# Multi-Device Coherence Audit

## Summary

Device → adapter routing works where it was deliberately built for multiple adapters. It breaks down at every entry point that predates the adapter split, and the model layer has no way to tell the rest of the app when the device list changes.

| Question | Verdict |
|---|---|
| Is it adapter-dependent? | **Partially.** The routing spine (grouping, sessions, result merge, register creation) is fully adapter-aware. Five entry points still assume a single Modbus adapter and device 1. |
| Is it coherent everywhere? | **No.** Ownership is resolved in three different places with three different rules, and the settings dialog mutates the model before the user presses OK. |
| Is the work finished? | **Not yet.** `deviceListChanged()` is emitted six times and connected zero times. Two pre-adapter UI files are still compiled into the binary. |

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

Where: `src/models/device.cpp:6` · `src/models/settingsmodel.cpp:141` · `src/communication/adapterpoll.cpp:218`

### F2 — MBC import is hardcoded to device 1 and gated on the wrong adapter [High]

`MbcRegisterData::toExpression()` emits `${address}` or `${address:type}` — never an `@deviceId`. Every imported register therefore lands on device 1, whichever adapter happens to own it.

The gate is equally loose: `isMbcCompatible()` returns true if *any* registered adapter reports the capability, so the import is enabled even when device 1 belongs to an adapter that does not support MBC. There is no device picker in the import flow at all.

Where: `src/MbcInterface/mbcregisterdata.cpp:181` · `src/models/settingsmodel.cpp:220` · `src/controllers/scopecontroller.cpp:237`

### F3 — The default new register always targets the first device [Medium]

`RegisterDialog::defaultExpressionManager()` picks `deviceList().first()`'s adapter, and `requestDefaultExpression()` then calls `buildExpression(fields, type, 0)`. Zero means "omit deviceId" on the wire, so the returned expression has no `@N` and resolves to device 1 on parse.

When the lowest device ID is not 1 — entirely possible after deleting device 1 — the default register silently points at a device that does not exist, using a schema fetched from a different adapter.

Where: `src/dialogs/registerdialog.cpp:172` · `src/dialogs/registerdialog.cpp:239` · `src/ProtocolAdapter/adapterclient.cpp:200`

### F4 — Adapter diagnostics are indistinguishable between adapters [Medium]

`AdapterManager::onAdapterDiagnostic()` forwards the message to the shared `scope.comm.adapter` category with no adapter ID attached, even though the manager knows its own `_adapterId`. `AdapterClient`'s own warnings do include it.

With two adapters running, the diagnostic log interleaves both streams with no way to tell them apart — precisely when you need it most.

Where: `src/ProtocolAdapter/adaptermanager.cpp:146`

### F5 — Communication statistics are aggregate only [Low]

Success and error counts are accumulated per register across all adapters, and median poll time is derived from graph 0's series. Nothing is broken by this, but a user with two adapters cannot see which one is failing or which one is slowing the cycle down.

Where: `src/communication/communicationstats.cpp:22`

---

## 4. Coherence breaks across layers

**Root cause:** Device ownership is decided in three places with three different rules: `reconcileDevicesWithAdapters()` (stored-config priority), `AdapterDeviceSettings`'s constructor (prune, then reconcile, then dedupe by iteration order), and `ProjectFileHandler::applyDeviceSettings()` (file wins, fall back to `"modbus"`). Each is individually defensible; together they make "who owns device N?" depend on which one ran last.

### F6 — `deviceListChanged()` has no consumers [High]

The signal is emitted from six call sites in `SettingsModel` — including the reconciliation path, where a comment explains at length why an already-known device changing owner needs its own notification. Nothing in `src/` connects to it. The only reference outside the model is a `QSignalSpy` in a test.

So the notification is produced with care and then discarded. Concretely: `ExpressionStatus` re-validates only on `added` and `expressionChanged`, so deleting a device never re-marks the registers that referenced it as `UNKNOWN_DEVICE` — they stay green and fail at poll time. `AddRegisterWidget::populateDeviceCombo()` runs once in the constructor and never refreshes.

Where: `src/models/settingsmodel.h:71` · `src/util/expressionstatus.cpp:17` · `src/dialogs/addregisterwidget.cpp:63`

### F7 — The settings dialog mutates the model before OK, and Cancel doesn't undo it [High]

Adapter config is applied transactionally through `acceptValues()`, called only from `SettingsDialog::done()` on `Accepted`. The device list is not. `handleAddTab()` calls `addDevice()` immediately, `handleCloseTab()` calls `removeDevice()` immediately, `DeviceConfigTab::onNameChanged()` writes the name on every keystroke, and `onAdapterChanged()` reassigns ownership on selection.

The leak is known — a test comment reads *"addNewDevice() → ID 2; leaks into model on cancel"* — and the prune step at the top of the next dialog open partially masks it. Renames and adapter reassignments are not masked at all: they survive Cancel permanently, because the pruner only removes devices that no adapter config mentions.

Where: `src/dialogs/adapterdevicesettings.cpp:204` · `:220` · `src/customwidgets/deviceconfigtab.cpp:91` · `:103` · `tests/dialogs/tst_adapterdevicesettings.cpp:608`

### F8 — Pressing OK marks every adapter as explicitly configured [Medium]

`AdapterDeviceSettings::acceptValues()` loops over *all* valid adapters and calls `setAdapterCurrentConfig()` for each, writing an empty `devices` array for adapters with no tabs. That setter unconditionally sets `hasStoredConfig = true`.

`hasStoredConfig` is the load-bearing input to reconciliation's priority ordering and to `buildCurrentAdapters()`'s export decision. After one trip through Settings, the flag no longer means "the user configured this adapter" — it means "the user once opened Settings" — and the `"modbus"`-first tie-break, which only applies within the unconfigured group, is permanently disabled.

Where: `src/dialogs/adapterdevicesettings.cpp:332` · `src/models/settingsmodel.cpp:244` · `src/importexport/projectfilehandler.cpp:170`

### F9 — The device limit warns, then silently truncates, and nothing gates Start [Medium]

Exceeding an adapter's `devices.maxItems` produces an orange label and nothing else — the tab is created, the device enters the model, and the config is stored. At session start `configForWire()` drops devices from the *end* of the array, which after `sortPagesByDeviceId()` means the highest IDs.

Those devices remain in `SettingsModel`, so `buildAdapterGroups()` still routes their registers to the adapter, which was never told they exist. `ScopeController::start()` checks only that at least one register is active — not that every referenced device exists, is owned by a live adapter, and survived truncation. The integration test that covers this path passes an empty register list.

Where: `src/models/adapterdata.cpp:200` · `src/dialogs/adapterdevicesettings.cpp:229` · `src/controllers/scopecontroller.cpp:126` · `tests/integration/tst_dummydevicelimit.cpp:68`

### F10 — Opening the devices page deletes devices as a side effect [Medium]

Before building any tabs, `AdapterDeviceSettings`'s constructor removes every model device not named by some adapter's `effectiveConfig()`. This is how the F7 leak gets cleaned up, but it is unconditional and unrelated to accepting: merely navigating to the Devices page and pressing Cancel can permanently drop a device that a hand-edited or older project file introduced.

Where: `src/dialogs/adapterdevicesettings.cpp:51`

### F11 — The project file stores device→adapter twice and reads one [Low]

Each exported device carries both a numeric `adapterId` index into the adapters array and an `adapter.type` string. The parser reads and validates both, but `applyDeviceSettings()` uses only `adapterType`, falling back to `"modbus"` when it is empty. The index is write-only and can disagree with the string after any hand edit.

Where: `src/importexport/projectfilejsonexporter.cpp:97` · `src/importexport/projectfilejsonparser.cpp:212` · `src/importexport/projectfilehandler.cpp:304`

### F12 — A device's `connectionId` is never validated against the connections list [Low]

`connectionId` is rendered as a plain integer field from the schema, with no awareness that it references another array in the same config. Deleting a connection on the Connections page leaves every device that pointed at it dangling, with no warning on either page. This is the main linking field in a multi-device Modbus setup.

Where: `src/dialogs/adaptersettings.cpp:158` · `src/customwidgets/deviceconfigtab.cpp:110`

---

## 5. Runtime behaviour with more than one adapter

### F13 — A mixed idle/ready fleet deadlocks the start path [High] — Fixed

`AdapterHub::isAdapterReady()` requires *all* managers ready; `isAdapterIdle()` requires *all* idle. `startCommunication()` handles exactly those two cases:

```
if (isAdapterReady()) start;
else { wait; if (isAdapterIdle()) initAdapter(); }
// else: adapter is already initializing  <- single-adapter reasoning
```

When one adapter has crashed to `IDLE` and another is still `AWAITING_CONFIG`, neither predicate holds. The poll enters `WaitingForAdapter`, arms a single-shot connection to `adapterReady`, and calls nothing — and `adapterReady` only fires when `_pendingReadyAdapters` drains, which requires an `initAdapter()` or `stopSession()` that never comes. The state is reachable: any adapter process dying mid-session produces it, and `stopSession()` skips both idle and ready managers, so it persists across a stop/start.

**Fixed:** `AdapterHub::initAdapter()` is now idempotent and per-manager — it (re)starts only the managers currently `IDLE`, leaving ready/mid-handshake/active managers untouched. `AdapterPoll` no longer reasons about a fleet-wide idle predicate at all; the aggregate `isAdapterIdle()` was removed. Regression test: `tst_adapterhub.cpp::initAdapterReinitializesOnlyIdleManagers`.

Where: `src/communication/adapterpoll.cpp:62` · `src/ProtocolAdapter/adapterhub.cpp:102` · `:160` · `:177`

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

## 6. Dead code from the pre-adapter era

Four files implement a complete, earlier device editor that nothing instantiates. `src/CMakeLists.txt` uses `GLOB_RECURSE`, so they are compiled into every build.

| File | Status | Why it matters |
|---|---|---|
| `dialogs/devicesettings.{h,cpp}` | Unreferenced | Replaced by `AdapterDeviceSettings`. Sole caller of `SettingsModel::addNewDevice()` in `src/`. |
| `customwidgets/deviceform.{h,cpp,ui}` | Unreferenced | Sole caller of `updateDeviceId()`. Offers a device-ID spinbox — a capability the current UI deliberately forbids, since `DeviceConfigTab` forces `id` read-only to avoid collisions with existing IDs. |

Keeping them alive keeps two `SettingsModel` APIs alive with them. `addNewDevice()` and `updateDeviceId()` are unreachable from the shipping UI, are not adapter-aware (neither touches `adapterId`, and `updateDeviceId()` would silently break every expression referencing the old ID), and are still part of the public model surface.

One stale comment travels with them: `AdapterDeviceSettings::handleAddTab()`'s doc block says it "creates a SettingsModel device via `addNewDevice()`", but the code computes its own ID and calls `addDevice()` — a difference that matters, because the hand-rolled version also scans open tabs for unsaved IDs.

---

## 7. Test coverage

Reconciliation is well tested — but only through the dialog, and only for ownership. The gaps sit where the routing actually runs.

| Area | Coverage | Gap |
|---|---|---|
| `reconcileDevices…` | Indirect | No `tst_settingsmodel` exists. Every case runs through `tst_adapterdevicesettings`, so model-level behaviour can only be asserted via a widget. |
| AdapterPoll grouping | Single adapter | Every assertion indexes `_startCalls[0]`. Nothing exercises two groups, the index-preserving merge, partial arrival ordering, or F16's mismatch path. |
| Mixed adapter states | None | `tst_adapterhub` covers aggregate ready/idle, but no test drives one adapter to `IDLE` while another stays ready — the F13 deadlock. |
| Device limit | Config only | `tst_dummydevicelimit` confirms the session starts after truncation, using an empty register list — so it cannot catch registers aimed at a truncated device. |
| Cancel semantics | Partial | `cancelAndReopenDoesNotLeakDeviceIds` asserts IDs are reused, and documents the leak. Nothing asserts that a rename or adapter reassignment is rolled back — because neither is. |

---

## 8. Where to start

Ordered by how much they unblock, not by size.

1. **Give `deviceListChanged()` its consumers (F6).** Connect `ExpressionStatus` so device removal re-validates every expression, and refresh `AddRegisterWidget`'s device combo. This is the smallest change with the widest reach, and it makes F1's failure visible instead of silent.

2. **Fix the mixed-state start path (F13).** Have `AdapterHub` re-init only the managers that are actually idle, rather than making `AdapterPoll` reason about a fleet-wide predicate. A crashed adapter should not be able to wedge the Start button.

3. **Add a read-data timeout (F14).** Per-adapter, reusing the handshake-timeout machinery. Decide alongside it whether a timed-out adapter fails the session (current F15 policy) or degrades to invalid results for its own registers.

4. **Make the device list transactional (F7, F10).** Have `AdapterDeviceSettings` edit a working copy and apply it in `acceptValues()`, alongside the adapter configs it already applies there. That removes the leak, the lost renames, and the need for the destructive prune on open.

5. **Validate the session before starting it (F1, F9).** In `ScopeController::start()`, check that every referenced device exists, is owned by a live adapter, and survived `configForWire()` truncation. Refuse with a message naming the devices rather than letting the adapter fail obscurely.

6. **Introduce an explicit "unassigned" adapter ID (F1).** Stop letting `"modbus"` double as the default. Every silent misroute above depends on an unknown device resolving to a real adapter.

7. **Give MBC import a device (F2).** Add a device selector, emit `@deviceId` from `toExpression()`, and gate the action on the selected device's adapter rather than on any adapter.

8. **Stop `acceptValues()` flagging untouched adapters (F8).** Write only adapters that actually own tabs, so `hasStoredConfig` keeps meaning what reconciliation and the exporter assume it means.

9. **Delete the pre-adapter editor, and its model APIs with it.** `devicesettings`, `deviceform`, `addNewDevice()`, `updateDeviceId()`. Removing them makes "device IDs are immutable after creation" a property of the model, not a convention of one dialog.

10. **Add `tst_settingsmodel` and a two-adapter `AdapterPoll` test.** Reconciliation deserves direct coverage, and the fan-out/merge is the one piece of multi-device machinery with no multi-adapter test at all.
