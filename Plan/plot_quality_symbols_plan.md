# Plot symbols for per-sample data quality

## Context

`DataQuality::State` (`Good`/`Degraded`/`Invalid`/`NoValue`) and `DataQuality::Flags`
(`Substituted`/`Blocked`/`Overflow`/`OldData`) already exist as a full, hand-maintained
vocabulary (`src/util/result.h:36-85`) and are already threaded all the way from the
adapter wire protocol to the plotted data model:

- `AdapterClient::decodeDataPoint()` (`src/ProtocolAdapter/adapterclient.cpp:606-639`)
  turns each `adapter.readData` entry's `state`/`flags` into a `ResultDouble`
  (`Result<double>`, `src/util/result.h:277`).
- `GraphDataHandler::handleRegisterData()` (`src/datahandling/graphdatahandler.cpp:165-178`)
  evaluates each graph's expression and aggregates the quality of its inputs into a new
  `ResultDouble` per graph (`summarizeInputs`/`evaluateExpression`, `.cpp:62-152`).
- `GraphSample` (`src/models/graphdataseries.h:8-14`) already stores
  `DataQuality::Quality quality` alongside `timestamp`/`value`, and
  `GraphView::plotResults()` (`src/graphview/graphview.cpp:493-528`) already writes it in:
  `_pGraphDataModel->mutableDataSeries(graphIdx)->add(timeData, value, result.quality())`
  (line 517).
- `legend.cpp:298-301` already reacts to `state() == Invalid` by coloring the legend row
  red (`GraphDataModel::lightRed`, `src/models/graphdatamodel.cpp:11`) — so there is one
  prior-art UI surface for quality, just not the plot area.

**The gap is entirely in rendering.** The same `plotResults()` call that stores quality
into `GraphDataSeries` immediately throws it away when updating the live `QCPGraph`:
`_pPlot->graph(i)->addData(timeData, value)` (line 519) takes only the value. Same for the
full-rebuild path, `GraphView::loadGraphDataFromModel()` (`.cpp:310-328`), which builds
plain `timestamps`/`values` vectors and calls `pGraph->setData(timestamps, values, true)`
(line 327). No new data model or protocol work is needed — this plan is scoped
entirely to `src/graphview/`.

## Design

### Rendering mechanism: `QCPCurve` overlays, not a second `QCPGraph`

The background research for this plan (a `QCPGraph` "special samples" overlay) does not
fit this codebase as-is. `graphview.cpp` and `graphscaling.cpp` both iterate
`_pPlot->graphCount()` / `_pPlot->graph(i)` in over a dozen places
(`graphview.cpp:111,119,411,453,532,566,673,695,735,780,817,854,871`,
`graphscaling.cpp:76,89,117,147`) under the hard assumption that graph index `i` ==
`ActiveIdx(i)`, i.e. exactly one `QCPGraph` per active signal. This is enforced by
QCustomPlot itself: **every** `QCPGraph` instance — however constructed — registers
itself into the plot's internal `mGraphs` list via `QCustomPlot::registerGraph()`, called
from the `QCPGraph` constructor (`libraries/qcustomplot/qcustomplot.cpp:16287-16302`,
called at `qcustomplot.cpp:16300`/`34192`). `graphCount()` (`.cpp:14531-14534`) and
`clearGraphs()` (`.cpp:14518-14524`) only ever see `mGraphs`. So adding one more
`QCPGraph` per signal for quality markers — anywhere in the plot's graph list, including
appended at the tail — would silently break every one of those loops (e.g.
`highlightSamples()` at `.cpp:778-791` would apply the sample-highlight scatter style to
the marker overlays too, `clearResults()` at `.cpp:530-541` would call
`convertToGraphIndex(ActiveIdx(i))` with out-of-range indices, etc.).

`QCPCurve` (`libraries/qcustomplot/qcustomplot.h:5634`, impl at
`qcustomplot.cpp:22696+`) is a separate plottable class that is **not** tracked in
`mGraphs` — it only registers as a generic `QCPAbstractPlottable`. It supports
`setData(keys, values)`, `setLineStyle(QCPCurve::lsNone)`, `setScatterStyle(...)`, and
`removeFromLegend()` — everything the overlay needs — without being visible to any of the
`graphCount()`/`graph(i)` call sites above, and without being touched by
`_pPlot->clearGraphs()` (so it needs its own explicit lifetime management, which is
already the plan below).

**Use one `QCPCurve` per (active signal, non-`Good` state) as the quality-marker
overlay**, line style `lsNone`, scatter-only.

### Symbol/color mapping (v1 proposal — confirm before implementing)

| State | Shape | Color | Rationale |
|---|---|---|---|
| `Good` | *(none)* | — | no visual change for the common case; existing line + the existing zoom-gated `highlightSamples()` circle already cover it |
| `Degraded` | `QCPScatterStyle::ssTriangle` | orange `QColor(255,140,0)` | "caution" tier, visually distinct from Invalid's red |
| `Invalid` | `QCPScatterStyle::ssCross` | `GraphDataModel::lightRed` (`#f08080`) | reuses the color the legend already uses for `Invalid` (`legend.cpp:299`), so the same meaning reads the same color everywhere |
| `NoValue` | `QCPScatterStyle::ssSquare` (hollow) | `Qt::gray` | Today a `NoValue` sample is stored as `value == 0` (see `updateGraphs()`'s zero-padding at `.cpp:281-288`, and `plotResults()`'s `result.isUsable() ? result.value() : 0` at line 514) — indistinguishable from a genuine `0.0` reading. A marker here is a real, incidental bug fix, not just decoration. |

Marker size ~8px. All overlay curves live on a dedicated `QCPLayer` created above
`"topMain"` (the layer `bringToFront()` already uses, `.cpp:422`), so markers are never
hidden by whichever signal is currently brought to front.

Flags (`Substituted`/`Blocked`/`Overflow`/`OldData`) are **not** given their own shapes in
v1 — surfaced as tooltip text only (see Phase 2 below), to avoid a combinatorial symbol
explosion. `Degraded` + flags renders as the single `Degraded` triangle.

### Positional/index safety

Because overlay curves are `QCPCurve`s, not `QCPGraph`s, they are invisible to every
existing `_pPlot->graphCount()`/`graph(i)` loop — no existing code needs to change to stay
correct. The new `GraphQualityMarkers` component (below) owns its own bookkeeping,
keyed by `GraphIdx`, independent of the main graphs' positional indices.

## Implementation

### 1. New: `src/graphview/graphqualitystyle.h`/`.cpp`

Pure mapping table, no Qt widget dependency beyond `QCPScatterStyle` (header-only from
qcustomplot, safe to unit test):

```cpp
namespace GraphQualityStyle {
    QCPScatterStyle scatterStyleFor(DataQuality::State state); // returns ssNone for Good
    QString displayText(DataQuality::State state, DataQuality::Flags flags); // for tooltips
}
```

### 2. New: `src/graphview/graphqualitymarkers.h`/`.cpp` (class `GraphQualityMarkers`)

Modeled directly on the existing sibling components `GraphMarkers`
(`src/graphview/graphmarkers.h`) and `GraphIndicators`
(`src/graphview/graphindicators.h`) — same constructor shape, same
`GraphDataModel*`/`ScopePlot*` dependencies, owned by `GraphView`.

```cpp
class GraphQualityMarkers : public QObject
{
public:
    explicit GraphQualityMarkers(GraphDataModel* pGraphDataModel, GuiModel* pGuiModel,
                                  ScopePlot* pPlot, QObject* parent = nullptr);

    void clear();                                    // drop all overlay curves + bookkeeping
    void addSignal(GraphIdx graphIdx);                // create the (up to 3) overlay curves for a newly plotted signal
    void rebuildFromSeries(GraphIdx graphIdx);        // full O(n) resync from GraphDataModel::dataSeries(graphIdx)
    void appendSample(GraphIdx graphIdx, double timestamp, double value,
                       const DataQuality::Quality& quality); // live single-point path
    void setAxis(GraphIdx graphIdx, const GraphData::valueAxis_t& axis);
    void setVisible(GraphIdx graphIdx, bool bVisible);
    void setEnabled(bool bEnabled);                   // global on/off (the new user toggle)

private:
    struct SignalOverlays { QCPCurve* degraded; QCPCurve* invalid; QCPCurve* noValue; };
    QMap<qint32 /* GraphIdx.v */, SignalOverlays> _overlays;
    ...
};
```

`rebuildFromSeries()` does a single pass over the series (not one pass per state) and
buckets each sample's `(timestamp, value)` into the vector matching its `quality.state`,
then calls `setData()` on the three curves once. `appendSample()` looks up the sample's
state and calls `addData()`-equivalent on the matching curve only (or no-ops for `Good`).

### 3. `src/graphview/graphview.h`/`.cpp` — wire it in

- Constructor/destructor: create/destroy `_pGraphQualityMarkers` alongside
  `_pGraphMarkers`/`_pGraphIndicators` (`.cpp:82`, `.cpp:95` today).
- `updateGraphs()` (`.cpp:234-303`): call `_pGraphQualityMarkers->clear()` next to
  `_pGraphMarkers->clearTracers()`/`_pGraphIndicators->clear()` (line 236-238, before
  `_pPlot->clearGraphs()`); inside the `foreach (graphIdx, activeGraphList)` loop
  (line 262-295), after `loadGraphDataFromModel(graphIdx, pGraph)` (line 291) call
  `_pGraphQualityMarkers->addSignal(graphIdx)` then `rebuildFromSeries(graphIdx)`.
- `clearGraph()` (`.cpp:188-229`): both branches end by either clearing the series
  entirely (line 203-206) or zeroing `.value` while keeping timestamps (line 213-221,
  which must also reset `it->quality` to `DataQuality::Quality{}` there — currently it
  doesn't touch quality at all, a pre-existing gap this feature would otherwise expose as
  stale markers surviving a "clear graph"). After either branch, call
  `_pGraphQualityMarkers->rebuildFromSeries(graphIdx)`.
- `plotResults()` (`.cpp:493-528`): right after `_pPlot->graph(i)->addData(timeData,
  value)` (line 519), add
  `_pGraphQualityMarkers->appendSample(graphIdx, timeData, value, result.quality())`.
- `addData()` (`.cpp:432-465`, bulk/import path): `GraphDataSeries::setSamples()`
  (`graphdataseries.h:38`) has no quality-carrying overload, so imported data has no
  per-sample quality today (see Open Questions). Call
  `_pGraphQualityMarkers->rebuildFromSeries(graphIdx)` per signal anyway, so overlays are
  at least cleared/consistent rather than stale from a previous session.
- `changeGraphAxis()` (`.cpp:350-361`): also call
  `_pGraphQualityMarkers->setAxis(graphIdx, _pGraphDataModel->valueAxis(graphIdx))` so
  markers move with their signal when it switches primary/secondary axis.
- `changeGraphColor()` (`.cpp:334-344`): deliberately **not** touched — marker colors are
  fixed per quality state, not per signal, so the same shape/color always means the same
  thing across every trace.
- `handleGraphVisibilityChange()` (`.cpp:471-484`): call
  `_pGraphQualityMarkers->setVisible(graphIdx, bShow)`.
- `clearResults()` (`.cpp:530-541`): after the existing loop, call
  `_pGraphQualityMarkers->clear()` then re-`addSignal()`/`rebuildFromSeries()` for each
  active graph (mirrors what the loop already does for the main graphs, just via
  `GraphIdx` instead of raw plot index — note this function currently loops
  `_pPlot->graphCount()` directly, which stays correct since overlays are invisible to
  `graphCount()`).
- New slot `updateQualityMarkersVisibility()`: calls
  `_pGraphQualityMarkers->setEnabled(_pGuiModel->showQualityMarkers())`.

### 4. `src/models/guimodel.h`/`.cpp` — new toggle, mirroring `highlightSamples` exactly

- `.h`: `bool showQualityMarkers() const;` / `void setShowQualityMarkers(bool);` /
  `signal showQualityMarkersChanged();` / `bool _bShowQualityMarkers{true};` (pattern at
  `guimodel.h:53,82,101`).
- `.cpp`: getter/setter mirroring `highlightSamples()`/`setHighlightSamples()`
  (`.cpp:87-99`); emit the new signal from `triggerUpdate()` (`.cpp:69-84`) alongside
  `highlightSamplesChanged()`.
- Not persisted to `SettingsModel`/project file — confirmed `highlightSamples` itself
  isn't persisted either (transient GUI/view state), so this stays consistent.

### 5. MainWindow UI wiring (mirrors `actionHighlightSamplePoints` exactly)

- `mainwindow.ui`: new `QAction actionShowQualityMarkers` in the same View menu as
  `actionHighlightSamplePoints`.
- `mainwindow.cpp`: `connect(actionShowQualityMarkers, toggled, _pGuiModel,
  setShowQualityMarkers)` (pattern at line 184); `connect(_pGuiModel,
  showQualityMarkersChanged, _pGraphView, updateQualityMarkersVisibility)` (pattern at
  line 217-218); initialize `actionShowQualityMarkers->setChecked(true)` where
  `actionHighlightSamplePoints` is initialized (line 520).

## Testing

`GraphView` has no existing unit tests (`tests/` has no `graphview/` directory) because it
owns a live `ScopePlot`/`QCustomPlot` widget — this plan does not change that.

- **New `tests/graphview/tst_graphqualitystyle.cpp`**: pure table-driven tests of
  `GraphQualityStyle::scatterStyleFor()`/`displayText()` — no widget needed, follows the
  `tst_*.cpp` convention.
- **`tests/models/tst_graphdataseries.cpp`**: already covers `GraphSample.quality` — no
  change needed.
- **`tests/datahandling/tst_graphdatahandler.cpp`**: already covers expression-quality
  aggregation — no change needed.
- **Manual verification** (required regardless, and the only option in a cloud/web
  session per `CLAUDE.md`): run the app against `DummyAdapter` configured to emit
  `Degraded`/`Invalid`/`NoValue` samples — `tests/integration/tst_dummyadapterquality.h/.cpp`
  already shows how to drive this at the protocol level — and visually confirm markers
  render, track pan/zoom, follow axis switches, and respect the new toggle.

## Open questions (flagged, not blocking)

1. Exact shapes/colors above are a first proposal for confirmation, not a final spec.
2. Imported/loaded project files (CSV, `.mbs`) have no per-sample quality today
   (`GraphDataSeries::setSamples()` is a 2-arg, quality-less overload) — persisting
   quality through file formats is a separate, out-of-scope change; overlays will simply
   be empty after a reload until that exists.
3. Per-flag sub-symbols (e.g. a different shape for `Substituted` vs. `OldData`) are
   deferred to tooltip text in v1 — revisit if users need to distinguish flags visually.
4. Phase 2 stretch: extend `paintTimeStampToolTip()`/`updateTooltip()`
   (`.cpp` — tooltip code) to show `GraphQualityStyle::displayText()` for a hovered
   non-`Good` sample.

## Rollout

1. `GraphQualityStyle` + its unit tests.
2. `GraphQualityMarkers` + wiring into `GraphView`'s lifecycle (the six call sites above).
3. `GuiModel` + `MainWindow` toggle.

Each step: build → test → quality → `code-reviewer` agent, per `CLAUDE.md` (skip
build/test/quality steps in a cloud/web container; CI covers them there).

## Verification

1. Build: `mkdir -p build && cmake -GNinja -S . -B build && ninja -C build` — clean under
   `-Wall -Wextra -Werror`.
2. Test: `ctest --test-dir build --output-on-failure` — all pass, including the new
   `tst_graphqualitystyle`.
3. Quality: `clang-format`, `./scripts/run_clang_tidy.sh`,
   `./scripts/run_clazy.sh` against every new/changed file — no violations.
4. Manual: DummyAdapter run as described under Testing.
5. `@agent-code-reviewer` on the full diff before considering the work done.
