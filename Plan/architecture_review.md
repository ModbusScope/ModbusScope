# Codebase Review: Architecture & Maintainability

> **Scope:** Full codebase review with focus on architecture and maintainability.

---

## 1. Critical Issues

### 1.1 Unsafe object lifetime in `MainApp`
**File:** `src/mainapp.cpp:48–58`

All six models are allocated without a Qt parent. The destructor manually deletes `MainWindow` _first_ (line 50), then the models (lines 52–57). At the moment `MainWindow` is deleted, it tears down widget children — but all model signals are still live. Qt disconnects them safely during `QObject::~QObject`, so there is no crash today, but the order is fragile and non-obvious. Any future eager access to a model from a widget destructor would be a use-after-free.

**Fix:** Pass `this` as parent to all model constructors so Qt owns and destroys them in correct reverse-construction order. `MainWindow` should be deleted last, or (better) also parented to `this`.

---

### 1.2 Static shared state in `QMuParser`
**File:** `src/datahandling/qmuparser.h:55`

```cpp
static ResultDoubleList _registerValues;  // shared across ALL instances
```

All `QMuParser` instances read from the same static buffer. The design is documented but still presents risks:
- Not protected by a mutex — unsafe the moment any background thread is introduced.
- Global state persists between unit tests, causing spurious test interactions.
- Evaluating two independent expression sets requires serialized access even if they are logically unrelated.

**Fix:** Pass register values as a parameter to `evaluate(ResultDoubleList&)` rather than via static state. `GraphDataHandler` already has the values at call time.

---

### 1.3 `GraphDataModel` mixes four unrelated concerns
**File:** `src/models/graphdatamodel.h` / `graphdatamodel.cpp`

The class simultaneously acts as:
1. `QAbstractTableModel` — row/column UI model with MIME drag-drop support
2. Graph data repository — owns `QList<GraphData>` and exposes 10+ per-graph getters/setters
3. Communication statistics store — `setCommunicationStartTime`, `setCommunicationStats`, `setMedianPollTime` etc. (lines 76–79)
4. Active-graph index cache — maintains a separate `_activeGraphList` that must be kept in sync

This makes the class ~700 lines, hard to test in isolation, and difficult to extend one concern without risking the others.

**Recommended split:**
- Keep `GraphDataModel : QAbstractTableModel` for the table view.
- Extract communication timing/stats into `GraphDataModel` or a dedicated value struct, owned by `CommunicationStats` and signalled to whomever needs it.

---

## 2. High-Priority Issues

### 2.1 `CommunicationStats` directly mutates `GraphDataModel`
**File:** `src/communication/communicationstats.cpp:43,48–50,65,71–73`

`CommunicationStats` calls setters on `GraphDataModel` (e.g. `setCommunicationStats`, `setCommunicationStartTime`, `setMedianPollTime`, `setCommunicationEndTime`) directly. This is a dependency-inversion violation — a lower-level communication component drives changes in the data model.

**Fix:** `CommunicationStats` should emit signals (`statsUpdated(successes, errors)`, `timingUpdated(median)`) and `GraphDataModel` (or a coordinator) should listen and apply them.

---

### 2.2 `GraphDataModel::dataMap()` leaks mutable internals
**File:** `src/models/graphdatamodel.h:58`

```cpp
QSharedPointer<QCPGraphDataContainer> dataMap(quint32 index);
```

Returns a non-const shared pointer to internal `QCPGraphDataContainer`. Any caller can call `.clear()` or append data without the model knowing, bypassing all signals. `CommunicationStats::updateTimingInfo` (communicationstats.cpp:20,26) relies on this to read timing data.

**Fix:** Return `QSharedPointer<const QCPGraphDataContainer>` for read-only callers. For write access (only `GraphView`), provide a dedicated model method with proper notification.

---

### 2.3 `SettingsModel::deviceSettings()` returns mutable raw pointer
**File:** `src/models/settingsmodel.h:25`

```cpp
Device* deviceSettings(deviceId_t devId);
```

Callers can freely mutate the returned `Device` without the model emitting any change signal. Ownership is also ambiguous — nothing in the API indicates whether the caller should delete this pointer.

**Fix:** Return `const Device*` for read access; add explicit mutator methods on `SettingsModel` that emit the appropriate signals.

---

### 2.4 `MainWindow` is a god class
**File:** `src/dialogs/mainwindow.cpp`

- Constructor is 160+ lines, wires 40+ signal/slot connections.
- Owns and coordinates: `GraphDataHandler`, `ModbusPoll`, `DataFileHandler`, `ProjectFileHandler`, `ExpressionStatus`, `CommunicationStats`, `GraphView`, `Legend`, `StatusBar`, `MarkerInfo`, plus all model pointers.
- Directly calls `_pGraphDataModel->clear()` and `_pNoteModel->clear()` (lines 470–471) — business logic scattered into the view layer.

**Fix (incremental):** Extract a non-UI `ApplicationController` (or `ScopeController`) that owns the service objects (`ModbusPoll`, `GraphDataHandler`, `CommunicationStats`) and exposes coarse actions (`startScope()`, `stopScope()`, `clearData()`). `MainWindow` delegates to it rather than wiring everything directly.

---

### 2.5 `FileSelectionHelper` uses global/static model reference
**File:** `src/mainapp.cpp:36`, `src/util/fileselectionhelper.h/.cpp`

```cpp
FileSelectionHelper::setGuiModel(_pGuiModel);
```

A static setter injects a model pointer into a utility class globally. This is hidden global state that makes the class non-reentrant, untestable in isolation, and fragile in any future multi-window scenario.

**Fix:** Pass `GuiModel*` as a constructor parameter to classes that need it, or use the model directly where it is needed.

---

### 2.6 `ProjectFileHandler` created without Qt parent
**File:** `src/dialogs/mainwindow.cpp:69`

```cpp
_pProjectFileHandler = new ProjectFileHandler(_pGuiModel, _pSettingsModel, _pGraphDataModel);
```

No parent passed. If `MainWindow` is destroyed while the handler is alive (or vice versa), this is a manual delete responsibility. All similar service objects created in `MainWindow` should receive `this` as parent.

---

## 3. Medium-Priority Issues

### 3.1 Test coverage gaps
**Directory:** `tests/`

The following classes have **no test files at all**:
- `GraphDataModel` (only `GraphData` value type is tested)
- `SettingsModel`
- `GuiModel`
- `NoteModel`
- `CommunicationStats`
- `ModbusPoll` / `ModbusMaster`
- `MainWindow`

These are the most business-critical classes. Changes to `GraphDataModel` in particular (the central data store) have no regression safety net.

---

### 3.2 `GraphDataHandler` is an unnecessary `QObject`
**File:** `src/datahandling/graphdatahandler.h`

`GraphDataHandler` inherits `QObject` solely to expose one slot (`handleRegisterData`) and emit one signal (`graphDataReady`). The slot just transforms `ResultDoubleList → ResultDoubleList` and the signal is a fan-out. This could be a plain function call or a non-QObject with `std::function` callbacks, eliminating the metaclass overhead and making it trivially testable without the event loop.

---

### 3.3 Output-parameter style in `activeGraphIndexList()`
**File:** `src/models/graphdatamodel.h:90`

```cpp
void activeGraphIndexList(QList<quint16>* pList);
```

The out-parameter pattern is less readable and error-prone compared to returning by value. Modern Qt and C++ can return `QList<quint16>` efficiently.

**Fix:**
```cpp
QList<quint16> activeGraphIndexList() const;
```

---

### 3.4 `triggerUpdate()` pattern hides initialization logic
**Files:** `src/mainapp.cpp:43–45`, model headers

```cpp
_pGuiModel->triggerUpdate();
_pSettingsModel->triggerUpdate();
_pDataParserModel->triggerUpdate();
```

A "trigger update" method that re-emits all signals is a workaround for the fact that signals emitted during construction are lost (no listeners connected yet). This is fragile — omitting a `triggerUpdate()` call silently leaves the UI in a wrong initial state.

**Fix:** Use property binding, connect before constructing (swap construction/connection order), or rely on `QMetaObject::invokeMethod(Qt::QueuedConnection)` for a single deferred initialization signal.

---

### 3.5 Signal naming inconsistency in `GraphDataModel`
**File:** `src/models/graphdatamodel.h:110–112`

```cpp
void moved();
void added(const quint32 idx);
void removed(const quint32 idx);
```

These three use past-tense verbs, inconsistent with the rest of the codebase's `XxxChanged()` convention. `added` and `removed` also differ from each other in that `moved()` passes no index.

---

### 3.6 No bounds checking on `_graphData[index]`
**File:** `src/models/graphdatamodel.cpp` (multiple locations)

Accesses like `_graphData[index]` and `_graphData.at(index)` are spread through the model without pre-condition checks. Invalid indices will crash silently. At minimum, add `Q_ASSERT(index < static_cast<quint32>(_graphData.size()))` in each accessor.

---

## 4. Low-Priority / Code Quality

| # | Issue | Location |
|---|-------|----------|
| 4.1 | `QDoubleValidator` allocated without parent (memory leak if widget init fails mid-way) | `src/customwidgets/scaledock.cpp:14` |
| 4.2 | `VersionDownloader` allocated inline as `new VersionDownloader()` and passed to `UpdateNotify` — ownership unclear | `src/dialogs/mainwindow.cpp:204` |
| 4.3 | `dynamic_cast<QApplication*>(QApplication::instance())` — fragile; use `qobject_cast` or `qApp` | `src/dialogs/mainwindow.cpp:196` |
| 4.4 | `_bFlag` prefix used inconsistently — some booleans use `_b`, others don't | Various |
| 4.5 | Communication stats fields (`_startTime`, `_endTime`, `_successCount`, `_errorCount`, `_medianPollTime`) live inside `GraphDataModel` with no logical grouping | `src/models/graphdatamodel.h:125–129` |
| 4.6 | `GraphData` uses `QSharedPointer<QCPGraphDataContainer>` internally, but it's only ever owned by one `GraphData` — `std::unique_ptr` or a plain value would be clearer | `src/models/graphdata.h` |
| 4.7 | `add()` is overloaded four times on `GraphDataModel` — consider named factory methods for clarity | `src/models/graphdatamodel.h:81–84` |

---

## 5. Positive Patterns Worth Preserving

- **Signal-driven UI updates** — models emit typed signals; views observe. No direct model→UI calls.
- **`GraphView` child ownership** — all sub-components (`GraphScale`, `GraphViewZoom`, `GraphMarkers`) created with `this` as parent. Correct.
- **Forward declarations** used consistently in headers to reduce compilation coupling.
- **`ExpressionParser`** — clean single-responsibility design; extracts register references from user expressions without knowing about the model layer.
- **Test infrastructure** — Google Test + Qt Test hybrid, mocks directory, good coverage of import/export and data-handling layers.
- **Include guards** — consistent `#ifndef`/`#define`/`#endif` pattern throughout.

---

## 6. Prioritised Action List

| Priority | Item | Files |
|----------|------|-------|
| P1 | Fix object lifetime / ownership in `MainApp` | `src/mainapp.cpp` |
| P1 | Remove static shared state from `QMuParser` | `src/datahandling/qmuparser.h/.cpp` |
| P1 | Split `GraphDataModel` responsibilities | `src/models/graphdatamodel.h/.cpp` |
| P2 | Decouple `CommunicationStats` from `GraphDataModel` via signals | `src/communication/communicationstats.cpp` |
| P2 | Make `dataMap()` return const; add proper write API | `src/models/graphdatamodel.h/.cpp` |
| P2 | Make `deviceSettings()` return `const Device*` | `src/models/settingsmodel.h` |
| P2 | Extract `ScopeController` from `MainWindow` | `src/dialogs/mainwindow.cpp` |
| P2 | Remove `FileSelectionHelper` static state | `src/util/fileselectionhelper.h/.cpp`, `src/mainapp.cpp` |
| P2 | Add tests for `GraphDataModel`, `SettingsModel`, `CommunicationStats` | `tests/models/`, `tests/communication/` |
| P3 | Convert `activeGraphIndexList()` to return-by-value | `src/models/graphdatamodel.h/.cpp` |
| P3 | Replace `triggerUpdate()` with proper deferred initialization | Model constructors, `src/mainapp.cpp` |
| P3 | Add `Q_ASSERT` bounds checks in `GraphDataModel` accessors | `src/models/graphdatamodel.cpp` |
| P3 | Standardise signal naming in `GraphDataModel` | `src/models/graphdatamodel.h` |
