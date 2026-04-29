# ModbusScope Test-Suite Audit

**Date:** 2026-04-29  
**Branch:** `claude/review-tests-improvements-G9OsU`  
**Scope:** All 34 `tst_*.cpp` files plus shared infrastructure (`tests/CMakeLists.txt`, `tests/mocks/`, helpers). No source files were modified.  
**Severity:** **blocker** = likely flaky/wrong/leak; **major** = real coverage/duplication/ownership concern; **minor** = style/naming/doc.

---

## Top Recommendations (read first)

1. **`tests/models/tst_mbcregistermodel.cpp`** — 22+ raw `MbcRegisterModel* pMbcRegisterModel = new MbcRegisterModel();` allocations are never deleted (lines 36, 50, 57, 71, 89, 113, 143, 236, 250, 330, 357, 399, 420, 445, 469, 493, 509, 517, 526, 540, 558, 560). Leak per slot. The neighbouring file `tst_mbcupdatemodel.cpp` already shows the correct pattern (`std::unique_ptr` in `init()`/`cleanup()`, line 25). Port it.

2. **`tests/ProtocolAdapter/tst_adapterclient.cpp:511,529`** — shutdown-timeout test uses a 50 ms internal timeout + `QTest::qWait(150)`. Will flake under CI load. Bump both to ~5× current values (e.g. 250 ms / 750 ms) or use `QSignalSpy::wait()` with a longer ceiling.

3. **`tests/ProtocolAdapter/tst_adapterprocess.cpp:50,73`** — fixed `QTest::qWait(500)` for subprocess startup. Replace with `QSignalSpy::wait()` against the readyRead/started signal so the test ends as soon as the event fires but tolerates slow boot.

4. **`tests/integration/tst_dummyadapter.cpp`** — depends on `DUMMY_ADAPTER_EXECUTABLE` being on disk and `LD_LIBRARY_PATH` being correct. If either is wrong every `QSignalSpy::wait()` runs the full 5–10 s timeout and the suite hangs. Add an explicit `add_dependencies` on the dummy-adapter target in CMake, and assert the binary is executable in `initTestCase()` with `QFileInfo::isExecutable()`.

5. **Test-data helpers fragmented across 5 files** (`tests/importexport/{csvdata,mbctestdata,presetfiletestdata,projectfilejsontestdata,projectfilexmltestdata}.{h,cpp}`) — inconsistent base classes (some `QObject`, some plain), no Doxygen on the static blobs, schema/format changes touch N files. Consolidate into a single `TestData` namespace or sibling classes with a shared base.

6. **`tests/dialogs/tst_adapterdevicesettings.cpp:635-668`** — `#if 0` block (`editingDeviceIdInSchemaFormUpdatesCorrectModelDevice`). Either re-enable, replace with `QSKIP("…")` and a tracker reference, or delete. Dead test code rots.

7. **`tests/importexport/tst_projectfilehandler.cpp:36-48`** — `writeTempProjectFile()` uses `setAutoRemove(false)` + manual `QFile::remove()` calls scattered in tests (lines 81, 93, 129, 206, 238, 257, 280, 294). If a `QCOMPARE` aborts mid-test, temp files leak to disk. Replace with a stack-allocated `QTemporaryFile` (auto-remove enabled) or `QTemporaryDir`.

---

## Cross-cutting Findings

- **`QVERIFY(a == b)` instead of `QCOMPARE(a, b)`** — `tst_mbcfileimporter.cpp:175`, `tst_expressionparser.cpp:277-278`, several spots in `tst_adapterdevicesettings.cpp`. `QCOMPARE` produces a usable diff on failure; `QVERIFY` only prints `false`.
- **Dead `if (ptr == nullptr) return;` after `QVERIFY(ptr != nullptr)`** — `tst_schemaformwidget.cpp:364-368`. `QVERIFY` already aborts the slot on failure; the `if` is unreachable.
- **Locale changes that leak across test classes** — `tst_projectfilejsonparser.cpp:18` and `tst_projectfilexmlparser.cpp:22` set `QLocale::setDefault(QLocale::Dutch, …)` in `initTestCase()` but never restore. `tst_util.cpp:11` does the same per-slot without documentation. Use RAII or restore in `cleanupTestCase()`.
- **Missing Doxygen on test helpers** (CLAUDE.md: `/*!`, `\brief`, `\param`) — `tst_settingsauto.cpp` (`prepareReference`, `processFile`), `tst_addregisterwidget.cpp` (`clickAdd`, `addRegister`), `tst_deviceconfigtab.cpp` (`setupTwoAdapters`), `tst_mbcregisterfilter.cpp` slot names, `tests/mocks/gmockutils.h` (`InitGoogleMock`/`CheckGoogleMock`).
- **Hardcoded sleeps as synchronisation** — `tst_adapterprocess.cpp:50,73`, `tst_adapterclient.cpp:529`, `tst_dummyadapter.cpp` (multiple). Prefer `QSignalSpy::wait(timeout)`.
- **Fixture duplication that should be `_data()` parameterisation** — `tst_schemaformwidget.cpp` (23 slots with type variants), `tst_adaptersettings.cpp:187-234` (8 ID-increment slots), `tst_graphdatahandler.cpp:150`, `tst_datafileparser.cpp` (12 slots).

---

## Findings by Area

### tests/models/

| File | Severity | Finding |
|------|----------|---------|
| `tst_mbcregistermodel.cpp` | **blocker** | 22+ `new MbcRegisterModel()` allocations never deleted — see Top #1 |
| `tst_mbcregistermodel.cpp` | **major** | Inline `QList<MbcRegisterData>` setup repeated at lines 91, 155, 252, 332, 359, 470, 494, 567 — extract beyond existing `fillModel()` (line 601) |
| `tst_graphdata.cpp` | **minor** | Only 2 test slots; `// TODO` at lines 19 and 59; color/style/active getters untested |
| `tst_diagnosticmodel.cpp` | **minor** | `_category` (line 11) rebuilt every `init()` — could be `const` class member |

**Clean:** `tst_adapterdata.cpp`, `tst_diagnostic.cpp`, `tst_mbcupdatemodel.cpp` (use as reference for `unique_ptr` pattern).

### tests/datahandling/

| File | Severity | Finding |
|------|----------|---------|
| `tst_expressionparser.cpp:277-278` | **minor** | `QVERIFY(actual == expected)` for `QList<DataPoint>` — use `QCOMPARE` |
| `tst_expressionparser.cpp` | **minor** | Inconsistent slot naming: `singleRegisterConn` vs `failureMulti` |
| `tst_graphdatahandler.cpp:16-19` | **major** | `qRegisterMetaType` called in `init()` per-slot — these are global; move to `initTestCase()` |
| `tst_graphdatahandler.cpp:150` | **major** | `graphDataTwice` is a clone of the previous slot — parameterise with `_data()` |

**Clean:** `tst_qmuparser.cpp` (already uses `_data()` slots well).

### tests/util/

| File | Severity | Finding |
|------|----------|---------|
| `tst_updatenotify.cpp:32-34` | **major** | `init()` computes expected strings via `QDate::currentDate().addDays(…)` — midnight flip can desync expected vs. actual mid-slot; inject a fixed clock |
| `tst_formatrelativetime.cpp:112-128` | **minor** | `_data()` branches on `QLocale::Belgium` territory; rows silently skipped on other hosts — use `QSKIP` with reason or parameterise |
| `tst_util.cpp:11` | **minor** | Dutch locale set per-slot without documentation |

**Clean:** `tst_expressionchecker.cpp`.

### tests/importexport/

| File | Severity | Finding |
|------|----------|---------|
| `tst_projectfilehandler.cpp:36-48` | **major** | Temp files leak on mid-test abort — see Top #7 |
| `tst_projectfilehandler.cpp` | **minor** | No malformed-JSON / missing-key / invalid-device-ID tests |
| `tst_projectfilehandler.cpp:91,206,238,257` | **minor** | Dereferences `deviceSettings(N)` without a prior null check |
| `tst_mbcfileimporter.cpp:175` | **minor** | `QVERIFY(list1.size() == list2.size())` → `QCOMPARE` |
| `tst_mbcfileimporter.cpp` | **major** | Error-path slots only verify the result list is empty; no distinction of parse errors or partial-parse state |
| `tst_projectfilejsonparser.cpp:18` | **minor** | Unrestored `QLocale::setDefault` — see cross-cutting |
| `tst_projectfilexmlparser.cpp:22` | **minor** | Same unrestored locale issue |
| `tst_projectfilexmlparser.cpp:296` | **minor** | `mkdir("subdir")` result not checked before `parseFile()` call — setup can fail silently |
| `tst_settingsauto.cpp` | **minor** | `prepareReference()` (229) and `processFile()` (242) lack Doxygen; no edge-case tests |
| `tst_projectfilejsonexporter.cpp` | **minor** | No failure-path tests (disk full, bad path) |
| `tst_datafileparser.cpp` | **minor** | No malformed-CSV tests; "simplified check" (lines 103-149) inspects only first/last/size |
| `tst_mbcregisterfilter.cpp` | **minor** | Missing per-slot Doxygen |

**Clean:** `tst_presetparser.cpp`, `tst_presethandler.cpp`.

### tests/ProtocolAdapter/

| File | Severity | Finding |
|------|----------|---------|
| `tst_adapterclient.cpp:511,529` | **blocker** | 50 ms timeout + 150 ms `qWait` — see Top #2 |
| `tst_adapterclient.cpp` | **minor** | No negative-input tests; ownership of `MockAdapterProcess*` (line 112) undocumented |
| `tst_adapterprocess.cpp:50,73` | **blocker** | Fixed `qWait(500)` — see Top #3 |
| `tst_adapterprocess.cpp` | **minor** | No `stop()`-before-`start()` or concurrent `sendRequest` tests |
| `tst_adaptermanager.cpp:10-25` | **major** | Static `capturedType()`/`capturedMessage()` shared across slots; early `QVERIFY` abort leaves stale state for next slot — use RAII guard around `qInstallMessageHandler` |
| `tst_framingreader.cpp` | **minor** | Error-path slots (`invalidHeader`, `incompleteHeader`) don't verify recovery; extend the recovery test at 150-166 to all error inputs |

### tests/integration/

| File | Severity | Finding |
|------|----------|---------|
| `tst_dummyadapter.cpp` | **blocker** | Binary/env dependency without build-system guard — see Top #4 |
| `tst_dummyadapter.cpp` | **minor** | No transient-error recovery test |
| `tst_dummyadapter.cpp` | **minor** | Slot names don't indicate subprocess is spawned |

### tests/customwidgets/

| File | Severity | Finding |
|------|----------|---------|
| `tst_schemaformwidget.cpp` | **major** | 23 type-variant slots should be one `_data()`-driven slot — see cross-cutting |
| `tst_schemaformwidget.cpp:365-368` | **minor** | Dead `if (typeCombo == nullptr) return;` after `QVERIFY` — see cross-cutting |
| `tst_schemaformwidget.cpp` | **minor** | No malformed-schema tests; `makeConditionalSchemaNoIfRequired` (106) lacks Doxygen |
| `tst_scaledock.cpp` | **major** | Single slot (`yAxis1SelectedOnBoot`); no tab-switching or interaction tests |
| `tst_scaledock.cpp` | **minor** | `_pScaleDock` member should default-init to `nullptr`; never `show()`n so visual state untested |
| `tst_deviceconfigtab.cpp` | **minor** | No signal-emission tests; `setupTwoAdapters` (51-55) lacks Doxygen |

### tests/dialogs/

| File | Severity | Finding |
|------|----------|---------|
| `tst_adapterdevicesettings.cpp` | **major** | 30 slots with 51 `findChild` null checks — extract `setupAdapterAndOpen()` / `findTabByIndex()` helpers |
| `tst_adapterdevicesettings.cpp:635-668` | **major** | `#if 0` dead test block — see Top #6 |
| `tst_adapterdevicesettings.cpp:612-614` | **minor** | Inline comments say "leaks into model on cancel" but the test at 619-631 asserts the opposite — stale/misleading; correct or remove |
| `tst_addregisterwidget.cpp` | **major** | `MockAdapterManager` ownership: if `AddRegisterWidget` adopts the raw pointer, manual `delete _pMockAdapterManager` at cleanup:77 is a double-free. Verify `src/dialogs/addregisterwidget.h` before any fix |
| `tst_addregisterwidget.cpp:159` | **minor** | `QTest::mouseClick` on a never-`show()`n radio button — click may not register |
| `tst_adaptersettings.cpp:187-234` | **minor** | 8 ID-increment slots should be one `_data()`-driven slot |
| `tst_adaptersettings.cpp` | **minor** | No tab-removal or type-mismatch tests |

---

## Shared Infrastructure

### `tests/CMakeLists.txt`

- **minor** — `add_xtest_mock` links full GTest+GMock kit to every mock-using test even if only `gmockutils` is needed. Acceptable, but worth a comment above the macro.
- **minor** — `CMAKE_AUTOUIC_SEARCH_PATHS` set without checking paths exist; silent failure if a folder is renamed. Add a `message(FATAL_ERROR …)` guard.

### `tests/mocks/`

| File | Severity | Finding |
|------|----------|---------|
| `gmockutils.h` | **minor** | `CheckGoogleMock()` declared but never defined in `gmockutils.cpp` — remove or implement |
| `gmockutils.h` | **minor** | Missing `\brief` Doxygen on both public functions |
| `mockgraphdatamodel.h` | **minor** | Mocks only one method; `communicationhelpers::addExpressionsToModel()` calls `add()`/`setExpression()` on a real model — verify if intentional and document |

**Clean:** `mockpresetparser.h`, `mockversiondownloader.h`.

### `tests/communication/communicationhelpers.h`

- **minor** — Static helpers that internally call `QVERIFY`/`QCOMPARE` are only safe inside QTest slots; add a Doxygen note.
- **minor** — No current consumers in any `tst_*.cpp` (no test files in `communication/` yet); consider deferring until communication tests land.

### Test-data helpers (`csvdata`, `mbctestdata`, `presetfiletestdata`, `projectfilejsontestdata`, `projectfilexmltestdata`)

- **major** — Inconsistent `QObject`/non-`QObject` base classes; no Doxygen on static blob members; format changes require updates in N independent files. Consolidate under one namespace/factory.

---

## Files with No Findings (Clean)

`tst_diagnostic.cpp`, `tst_diagnostic.h`, `tst_diagnosticmodel.h`, `tst_adapterdata.{cpp,h}`, `tst_graphdata.h`, `tst_mbcregistermodel.h`, `tst_mbcupdatemodel.{cpp,h}`, `tst_expressionchecker.{cpp,h}`, `tst_util.h`, `tst_formatrelativetime.h`, `tst_updatenotify.h`, `tst_qmuparser.cpp`, `tst_presetparser.cpp`, `tst_presethandler.cpp`, and all `CMakeLists.txt` files under `tests/`.

---

## Verification Notes

Six high-impact claims were confirmed by direct source read:

- ✅ **Confirmed** — `tst_mbcregistermodel.cpp` raw-`new` leaks (lines 36, 50, 57, 71, 89, 113 and more; no deletes).
- ✅ **Confirmed** — `tst_adapterclient.cpp:511,529` 50/150 ms timeouts.
- ✅ **Confirmed** — `tst_projectfilehandler.cpp:36-47` `setAutoRemove(false)` + manual `delete`.
- ✅ **Confirmed** — `tst_schemaformwidget.cpp:365-368` dead `if-return` after `QVERIFY`.
- ⚠️ **Demoted** — `tst_adapterdevicesettings.cpp:612-614` was reported as a "documented memory leak" / blocker. Direct read shows the second-session block (619-631) asserts `tabs->count() == 1` — device 2 did *not* leak. The inline comment is stale, not the code. Filed as **minor [misleading comments]**.
- ⚠️ **Conditional** — `tst_addregisterwidget.cpp:135-141` double-delete risk depends on whether `AddRegisterWidget` adopts the `AdapterManager*`. Needs verification of `src/dialogs/addregisterwidget.h`; filed as **major** pending confirmation.

---

## Suggested Fix Order

| Pass | Effort | Items |
|------|--------|-------|
| **1 — Quick wins** | ~2 h | Dead `if-after-QVERIFY`; stale comments; `QVERIFY→QCOMPARE` swaps; Doxygen on helpers; dead `CheckGoogleMock` declaration; `#if 0` test block |
| **2 — Memory hygiene** | ~0.5 d | Port `unique_ptr` pattern from `tst_mbcupdatemodel` to `tst_mbcregistermodel`; verify `tst_addregisterwidget` ownership; switch `tst_projectfilehandler` to `QTemporaryDir` |
| **3 — Flakiness** | ~0.5 d | Replace fixed `qWait` with `QSignalSpy::wait()` in adapter tests; `add_dependencies` for dummy adapter in CMake; restore locale in JSON/XML parser tests |
| **4 — Refactors** | ~1-2 d | Factor `setupAdapterAndOpen()` for `tst_adapterdevicesettings`; parameterise `tst_schemaformwidget` and `tst_adaptersettings` ID-increment slots; consolidate test-data helpers |
| **5 — Coverage** | open-ended | Malformed-input tests for parsers; error paths for project-file exporter; signal coverage for widget tests; recovery tests for `tst_framingreader` |
