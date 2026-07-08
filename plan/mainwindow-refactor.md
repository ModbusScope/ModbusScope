# MainWindow Refactor Plan

## Problem

`src/dialogs/mainwindow.cpp` is a classic god class (~950 lines). The header
declares 30+ slots/methods, and the class directly owns 8 models plus ~12
collaborator objects. The constructor alone is ~200 lines of manual `connect()`
wiring. It mixes at least eight distinct responsibilities:

| # | Responsibility | Representative members |
| --- | --- | --- |
| 1 | Object graph construction & ownership | constructor (`new` everything, wire it together) |
| 2 | Signal/slot glue (model→view, view→handler) | constructor connect blocks |
| 3 | Menu/action → command dispatch | `startScope`, `stopScope`, `showSettingsDialog`, `showAbout`, `showDiagnostic`, `selectImageExportFile` |
| 4 | Show/hide graph submenu management | `rebuildGraphMenu`, `handleGraph{Color,Label,Visibility}Change`, `menuShowHideGraphClicked` |
| 5 | GUI state machine (enable/disable actions per state) | `updateGuiState`, `projectFileLoaded` |
| 6 | Scope session orchestration (the actual app logic) | `startScope`, `stopScope`, `clearData`, `onRegisterDataReady` |
| 7 | File / project / drag-drop / CLI handling | `handleFileOpen`, `handleCommandLineArguments`, `dropEvent`, `dragEnterEvent` |
| 8 | Window/OS events | `keyPressEvent`, `keyReleaseEvent`, `closeEvent`, `appFocusChanged` |

Key observation for the CLI goal: responsibilities **#6 and #7 are not UI code**.
`startScope`/`stopScope`/`clearData`/`onRegisterDataReady`/`handleFileOpen` are
pure application logic (drive the poll, run expressions, feed the graph data
handler, export to file) that happen to live inside a `QMainWindow`. A
headless/CLI mode cannot reuse any of it today because it is welded to `_pUi`
and the widget.

## Goal

Split `MainWindow` into focused collaborators and establish a clean seam
between UI and a future headless/CLI mode. After the refactor, `MainWindow`
should keep only: construct child controllers, launch dialogs, handle
window/OS events, and forward user intent. Target ~250–300 lines.

## Target architecture

```text
MainApp
 ├── ScopeController        (headless: session + file/data logic + adapter)   <- reusable by CLI
 ├── MainWindow (GUI front-end, thin)
 │     ├── GuiStateController     (GuiState -> action enablement)
 │     ├── GraphMenuController    (show/hide submenu <-> GraphDataModel)
 │     └── window events + dialog launching only
 └── (future) ScopeCliRunner      (headless front-end)   <- reuses ScopeController
```

## Work items

### 1. Extract CLI parsing into `MainApp`  (low risk, unblocks headless)

- Move `handleCommandLineArguments` (`QCommandLineParser` setup) out of the
  `MainWindow` constructor up to `MainApp`/`main.cpp`.
- Parse once, pass a typed result (project file path, room for future
  `--headless`, `--export` flags) down to the front-end.
- `MainApp` already constructs all models centrally — natural home for the
  GUI-vs-CLI decision.

### 2. Extract `GuiStateController`  (low risk, highly testable)

- Move `updateGuiState` (~87 lines) and `projectFileLoaded` into a class that
  maps `GuiState` -> action enable/disable.
- Takes the `Ui::MainWindow*` (or a small interface) and `GuiModel`.
- Add unit tests (fits the repo "add a test when fixing" convention).

### 3. Extract `GraphMenuController`  (self-contained cluster)

- Move `rebuildGraphMenu`, `handleGraphColorChange`, `handleGraphLabelChange`,
  `handleGraphVisibilityChange`, `menuShowHideGraphClicked`,
  `handleGraphsCountChanged`.
- Owns `_pGraphShowHide` + `_pOverlayLabel`.
- Collapse the repeated `isActive -> convertToActiveGraphIndex -> bounds-check`
  boilerplate into one helper.

### 4. Extract `ScopeController`  (the big one — the UI/CLI seam)

- A `QObject` (no widget dependency) owning the session lifecycle and non-UI
  collaborators: `AdapterPoll`, `GraphDataHandler`, `CommunicationStats`,
  `DataFileHandler`, `ProjectFileHandler`, `ExpressionStatus`, relevant models.
- Move in `startScope`, `stopScope`, `clearData`, `onRegisterDataReady`,
  `handleFileOpen` and the register-data plumbing.
- Expose intent as a small API: `start()`, `stop()`, `clear()`,
  `openFile(path)`, `openProject(path)`, `exportData(path)`.
- Emit signals (`started`, `stopped`, `dataReady`, `error`) instead of poking
  widgets. `MainWindow` becomes a consumer; a future `ScopeCliRunner` is
  another consumer.

### 5. Constructor cleanup

- Split the ~120 lines of `connect()` calls into `setupConnections()` /
  `setupMenuActions()` helpers (or a `MainWindowWiring` collaborator).

## Sequencing

Each step must build, pass tests, and pass quality independently.

1. CLI parsing -> `MainApp`  (small, isolating, unblocks headless)
2. Extract `GuiStateController`  (pure, unit-testable)
3. Extract `GraphMenuController`  (self-contained)
4. Extract `ScopeController`  (big; do last, after the class is thinned)
5. Constructor cleanup (`setupConnections()`)

Steps 1–3 are low-risk and deliver value immediately. Step 4 is where the real
UI/CLI separation lands.

## Definition of done (per step)

After every step, run all four sub-agents in order and all must pass:
**build -> test -> quality -> review**.
