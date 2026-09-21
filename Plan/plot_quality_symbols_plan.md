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

### Prerequisite fix (already applied, separate commit)

While mapping this out, `GraphView::clearGraph()`'s "several active graphs" branch
(`.cpp:210-227`) turned out to zero a sample's `value` while leaving its `quality`
untouched, so a sample that used to be `Invalid`/`Degraded` kept reporting that stale
quality after a "clear graph" even though its value is now a meaningless placeholder.
This is fixed ahead of this feature (`it->quality` reset to
`DataQuality::Quality{DataQuality::State::NoValue, DataQuality::Flag::NoFlags}` alongside
`it->value = 0.0`, matching the padding samples `updateGraphs()` already creates for the
same "no measurement for this timestamp" case at `.cpp:281-288`), with a regression test
in the new `tests/graphview/tst_graphview.cpp`. Fixing it separately, before the marker
feature exists, means the feature doesn't inherit a latent data-correctness bug and the
fix's own diff/blame stays readable on its own.

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

> **Status: implemented.** Two deviations from the original design are marked below; both
> came out of the maintainability review and reduce the moving parts rather than add any.

### 1. `src/graphview/graphqualitymarkers.h`/`.cpp` (class `GraphQualityMarkers`)

Modeled on the existing sibling components `GraphMarkers` (`src/graphview/graphmarkers.h`)
and `GraphIndicators` (`src/graphview/graphindicators.h`) — same constructor shape, same
`GraphDataModel*`/`ScopePlot*` dependencies, owned by `GraphView`.

**Deviation 1 — no separate `graphqualitystyle.h`/`.cpp`.** The state→symbol mapping is a
file-static `scatterStyle()` in `graphqualitymarkers.cpp`. A file pair for ~15 lines of pure
mapping earned nothing once the tooltip work (its only other would-be consumer) was deferred,
and the marker tests assert the rendered scatter shapes end to end anyway, which covers the
mapping better than testing it in isolation would.

**Deviation 2 — one `rebuild()` instead of paired `clear()`/`addSignal()`/`rebuildFromSeries()`.**
The review flagged the biggest risk as six hand-synchronized call sites that a future change
could silently forget. `rebuild()` tears everything down and re-derives it from the model, so
it is idempotent and self-contained: every cold path just calls it, and there is no partial
state to keep in step. Only the live poll path stays incremental.

```cpp
void rebuild();  // full teardown + rebuild of every overlay from the model
void appendSample(GraphIdx graphIdx, double timestamp, double value, const DataQuality::Quality& quality);
void setAxis(GraphIdx graphIdx, GraphData::valueAxis_t axis);
void setVisible(GraphIdx graphIdx, bool bVisible);
```

`setAxis()`/`setVisible()` are kept only because they are O(1) where `rebuild()` would be
O(n) — `rebuild()` re-derives both from the model too, so they cannot drift out of sync.
Overlays are keyed by `GraphIdx` (which already has `operator<`), not by a raw int, keeping
the codebase's strong-index discipline.

`rebuild()` buckets each signal's samples in a single pass over its `GraphDataSeries` and
calls `setData()` once per overlay curve. Each curve is created with `lsNone`,
`QCP::stNone` (so marker clicks never steal a graph selection), `removeFromLegend()`, and a
dedicated `qualityMarkers` layer stacked above `topMain`.

### 2. `src/graphview/graphview.h`/`.cpp` — wire it in

- Constructor/destructor: create/destroy `_pGraphQualityMarkers` alongside
  `_pGraphMarkers`/`_pGraphIndicators`.
- `rebuild()` on the cold paths: `updateGraphs()` (after the padding loop, so padded
  `NoValue` samples are included), both branches of `clearGraph()`, `addData()` and
  `clearResults()`.
- `appendSample()` on the live poll path in `plotResults()`, right after the main graph's
  `addData()`.
- `setAxis()` from `changeGraphAxis()`, `setVisible()` from `handleGraphVisibilityChange()`.
- `changeGraphColor()` deliberately untouched — marker colors are fixed per quality state,
  not per signal, so a symbol always means the same thing across every trace.

Markers are always shown — no user-facing toggle in v1 (see Open Questions).

## Testing

`tests/graphview/tst_graphview.cpp` (added with the prerequisite fix above) covers both the
fix and the markers, driving a real `GraphView` + `ScopePlot` headlessly:

- `qualityMarkersBucketSamplesByState` — a mixed-quality series produces the right marker
  counts and coordinates per shape, and asserts `graphCount()` still equals the active-signal
  count, i.e. the overlays really are `QCPCurve`s and cannot corrupt the positional
  `graph(i)` loops.
- `qualityMarkersShowPaddedSamplesAsNoValue` — `updateGraphs()`'s zero-padding of a short
  series shows up as `NoValue` markers, the ambiguity this feature set out to fix.
- `qualityMarkersAppendLiveSample` — `plotResults()` routes a live `Invalid` result to the
  matching overlay only.
- `qualityMarkersFollowGraphVisibility` — overlays hide and show with their signal.

The remaining verification is visual and cannot be automated here: run the app against
`DummyAdapter` emitting `Degraded`/`Invalid`/`NoValue`
(`tests/integration/tst_dummyadapterquality.h/.cpp` shows how to drive it at the protocol
level) and confirm the markers read well at realistic data densities.

## Open questions (flagged, not blocking)

1. No user-facing on/off toggle in v1 — markers always render when present. Revisit only
   if real usage shows them visually noisy; adding a toggle later is a small, additive
   change (`GuiModel` property + one menu action), not a redesign.
2. Exact shapes/colors above are a first proposal for confirmation, not a final spec.
3. Imported/loaded project files (CSV, `.mbs`) have no per-sample quality today
   (`GraphDataSeries::setSamples()` is a 2-arg, quality-less overload) — persisting
   quality through file formats is a separate, out-of-scope change; overlays will simply
   be empty after a reload until that exists.
4. Per-flag sub-symbols (e.g. a different shape for `Substituted` vs. `OldData`) are
   deferred to tooltip text in v1 — revisit if users need to distinguish flags visually.
5. Phase 2 stretch: extend `paintTimeStampToolTip()`/`updateTooltip()`
   (`.cpp` — tooltip code) to show `GraphQualityStyle::displayText()` for a hovered
   non-`Good` sample.

## Rollout

1. ~~Prerequisite `clearGraph()` quality-reset fix + `tst_graphview.cpp`~~ — done.
2. ~~`GraphQualityMarkers` + wiring into `GraphView`'s lifecycle + marker tests~~ — done.

Each step: build → test → quality → `code-reviewer` agent, per `CLAUDE.md` (skip
build/test/quality steps in a cloud/web container; CI covers them there).

## Verification

1. Build: `mkdir -p build && cmake -GNinja -S . -B build && ninja -C build` — clean under
   `-Wall -Wextra -Werror`.
2. Test: `ctest --test-dir build --output-on-failure` — all pass, including
   `tst_graphview`.
3. Quality: `clang-format`, `./scripts/run_clang_tidy.sh`,
   `./scripts/run_clazy.sh` against every new/changed file — no violations.
4. Manual: DummyAdapter run as described under Testing.
5. `@agent-code-reviewer` on the full diff before considering the work done.
