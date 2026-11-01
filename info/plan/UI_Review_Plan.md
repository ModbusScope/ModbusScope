# UI Review: Implementation Plan

Implementation order for the findings in [UI_Review.md](UI_Review.md). Each step is one commit. Every step is reviewed and approved before it is implemented, and passes build, tests, quality checks and code review before it is committed.

All findings were verified against the current code; the evidence is listed per step.

The manual screenshot (`expression_dialog.png`) is out of scope; it is updated separately.

## Overview

| Step | Finding | Dialog | Size | Risk |
|---|---|---|---|---|
| 1 | Two add buttons with similar labels | Signals | Small | Low |
| 2 | Test-values labels and grouping | Compose expression | Small | Low |
| 3 | Redundant title and fixed editor height | Compose expression | Small | Low |
| 4 | Insert control looks like a command; help can overwhelm the workspace (steps 4 and 5 merged) | Compose expression | Medium | Low |
| 5 | *(merged into step 4)* | — | — | — |
| 6 | Sidebar hierarchy is implicit | Signals | Large | Medium |

Steps 2–4 all touch `expressionsdialog.ui`; doing them one by one keeps each diff readable. Step 6 is last because it carries the most risk.

---

## Step 1 — Distinct labels for the two add buttons

**Finding.** The button above the table reads **New signal** and switches the panel to add mode. The panel's button reads **Add signal** and adds the filled-in form to the table. The panel heading also reads **New signal**. Similar labels make it hard to predict which button starts the workflow and which one adds a row.

Evidence: `signalsdialog.ui` (`btnAdd`, "New signal"), `signaleditorpanel.ui` (`btnAdd`, "Add signal"; `lblTitle`, "New signal"), `SignalEditorPanel::applyMode()`.

**Intent.** One clear entry point and one clearly different commit action. The panel heading keeps naming the mode.

**Proposed change.**

| Where | Now | After |
|---|---|---|
| Button above the table | New signal | Add signal |
| Panel commit button | Add signal | Add to table *(or Add to list — to decide)* |
| Panel heading | New signal / Edit signal | unchanged |

- Labels only; no behaviour or object-name change.
- Update the doc comment in `signaleditorpanel.cpp` that names the button, and the test comments that refer to it.
- **Label choice:** the review suggests *Add to list*; the manual already says "the signal is added to the **table**", so *Add to table* is consistent with the docs.

**Tests.** New `TestSignalsDialog::addButtonsHaveDistinctLabels` that checks both labels.

**Docs.** `how-to/add-data-points.md` (steps 2 and 7, the reject note, "To add a new signal again…"), `tutorials/first-logging-session.md` (steps 4–6).

---

## Step 2 — Clear test-values labels, inputs and result grouped

**Finding.** The sample-testing area uses a section label **Example Input**, an editable column **Value**, and a separate label **Processed Output** in its own grid cell. The relation between inputs and result is unclear, and example values can be mistaken for live readings.

Evidence: `expressionsdialog.ui` (`label_5` "Example Input", `label_6` "Processed Output", grid rows 5–6), header labels set in the `ExpressionsDialog` constructor.

**Intent.** Make it obvious that these are test values you type in, and that the result is calculated from them.

**Proposed change.**

- Put the input table and the result in one group box titled **Test values** (replaces the "Example Input" label).
- Rename the editable column from **Value** to **Input value**.
- Rename **Processed Output** to **Calculated result**; it stays to the right of the table, inside the group.
- No behaviour change.

**Tests.** Check the group title, the column header and the result label.

**Docs.** `how-to/write-expressions.md`: step 4 and the error-handling paragraph (labels, and that these are test values, not live readings).

---

## Step 3 — Remove the duplicate title; let the editor resize

**Finding.** The window title **Compose expression** and the group-box title **Compose Expression** repeat the same idea. The expression editor has a fixed height of three lines: cramped for long formulas, and it cannot use extra space when the dialog is enlarged.

Evidence: `expressionsdialog.ui` (window title, `groupBox` title), `ExpressionsDialog::setupExpressionEditor()` (`setFixedHeight`, `cExpressionEditorLines = 3`).

**Intent.** One title layer, and an editor that grows with the dialog within sensible bounds.

**Proposed change.**

- Replace the titled `groupBox` with a plain container without frame or title. The **Expression** label above the editor stays.
- Editor: minimum height 3 lines, maximum height 10 lines, vertical size policy *Expanding*; extra dialog height is shared between the editor and the test-values table.
- Keep wrapping, the vertical scroll bar and the Enter handling.

**Tests.** The editor's minimum height is at least 3 lines, and the maximum is larger than the minimum and bounded.

**Docs.** None.

---

## Step 4 — One collapsible-section style (steps 4 and 5 merged)

**Findings.**
- **Insert data point…** looks like a command that opens another dialog, but it is a checkable button that expands an inline form. There is no visible expanded/collapsed state, and the form repeats the title "Insert data point".
- The help is an unbounded block below the composing area. Long adapter help can push the editor and the dialog buttons out of view, and **Show info** has the same command-versus-disclosure problem.

A first version that only turned Insert data point into a flat arrow button showed that it was still unclear where to click, because it looked different from the Test values group box and the Show info button.

Evidence: `expressionsdialog.ui` (`btnInsertDataPoint` checkable `QPushButton`, `insertGroupBox` title, `testValuesGroupBox`, `widgetInfo` with unbounded `lblInfo`, checkable `btnInfo` "Show info").

**Intent.** Insert data point, Test values and the help all use the same recognisable section header. Help is available on demand without taking over the dialog.

**Change.**
- New reusable `CollapsibleHeader` widget (`src/customwidgets`):
  - an arrow (right when collapsed, down when expanded) with a bold title, followed by a line across the row;
  - a click anywhere on the row toggles the section, and the header shows or hides one content widget.
- Three sections: **Insert data point** (collapsed), **Test values** (expanded) and **Expression help** (collapsed).
  - The group box and the Show info button are removed.
  - The lazy creation of the insert form stays.
- The help text sits in a `QScrollArea` with a maximum height of 15 text lines. The device selector stays above it.
- Tab order follows the visual order and includes the help scroll area.

**Tests.**
- Widget: the arrow and the content follow the state, and a click on the button and on the row toggles.
- Dialog: the start state of the three sections, and the help is inside the bounded scroll area.
- The existing insert and help tests keep passing.

**Docs.** `how-to/write-expressions.md`: expand **Insert data point**, Test values open by default, a sentence on **Expression help**.

---

## Step 5 — merged into step 4

---

## Step 6 — Sidebar organised into Signal, Data points and Expression

**Finding.** The Signals panel stacks name, data-point list, address fields, expression preview, axis, status and actions in one flat column. When a signal combines several data points, it is not clear which fields affect the selected data point and which affect the whole signal.

Evidence: `signaleditorpanel.ui` (flat `QVBoxLayout`: `lineName`, `lblDataPoints`, `listDataPoints`, `addressContainer`, `lblExpression`, `axisGroupBox`, `lblStatus`, `btnAdd`, `btnEditExpression`).

**Intent.** Group the fields by what they affect, and make the selected data point clear.

**Proposed change.**

- **Signal** section: name, and the Y1/Y2 axis choice on one row (replaces the separate *Y-axis* group box).
- **Data point(s)** section: the data-point list (only for multi-point signals) and the address editor. The title reads *Data point*, or *Data points* when the signal combines several; the hint reads "This signal combines N data points. Select one to edit it."
- **Expression** section: the read-only expression preview (with the edited data point in bold) and **Edit expression…**. The redundant "Expression:" caption in the preview is dropped. In add mode the section is hidden until a reference has been built.
- Status line and the add button stay below the sections. Tab order follows the visual order for the panel's own widgets.
- **Tab order (follow-up):** `QWidget::setTabOrder()` ignores `NoFocus` containers, and widgets created after the `.ui` widgets join the end of the focus chain. `DataPointEditorWidget::setTabOrderAfter()` places the device and address fields, and places them again after each form rebuild. The panel and the Compose expression insert form use it.

**Risk — investigated.**

1. Enter lost inside group boxes: Qt 6.8 (the version the project ships) has a bug. `QGroupBox::event` takes every key in the platform theme's `ButtonPressKeys`, even when the box is not checkable; Qt 6.9 added the `isCheckable()` check. GNOME lists Return/Enter there, the offscreen platform does not. The existing `addressContainer` group box already caused this in the released app: on GNOME, Enter in an address field did not add the signal (`enterInAddressFieldAddsSignal` failed under Wayland). The Compose expression insert form had the same cause until step 4 removed its group box. The sections therefore use plain containers with a header row (bold title and a line, like the collapsible headers in Compose expression) and no `QGroupBox`.
2. **Edit expression…** with height 0 only happens in a test with a panel that is never shown; the click test now shows the panel first.

**Tests.** The panel contains no `QGroupBox` (regression test for the Enter bug, which the offscreen platform cannot show); Expression section visibility in add and edit mode; Data point section hidden without data points; Enter still adds the signal; Edit expression still requests the dialog. Existing tests that compare the preview text are updated for the dropped caption.

**Docs.** `how-to/add-data-points.md`: refer to the Signal, Data point(s) and Expression sections in the add and edit steps.
