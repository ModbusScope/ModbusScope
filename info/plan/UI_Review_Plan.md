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
| 4 | Insert control looks like a command | Compose expression | Small | Low |
| 5 | Help can overwhelm the workspace | Compose expression | Medium | Low |
| 6 | Sidebar hierarchy is implicit | Signals | Large | Medium |

Steps 2–5 all touch `expressionsdialog.ui`; doing them one by one keeps each diff readable. Step 6 is last because it carries the most risk.

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

## Step 4 — Insert data point as a disclosure control

**Finding.** **Insert data point…** looks like a command that opens another dialog, but it is a checkable button that expands an inline form and stays pressed while open. There is no visible expanded/collapsed state. The form inside repeats the title "Insert data point".

Evidence: `expressionsdialog.ui` (`btnInsertDataPoint` checkable `QPushButton`, `insertGroupBox` title), `ExpressionsDialog::handleInsertDataPointToggled()`.

**Intent.** The control reads as "expand/collapse a section", and the expression editor stays the visually dominant element.

**Proposed change.**

- `btnInsertDataPoint` becomes a checkable `QToolButton` with text and arrow: right arrow when collapsed, down arrow when expanded. Label **Insert data point** (no ellipsis); the Alt+I mnemonic stays.
- Remove the repeated title from the form's group box; the frame stays to show what belongs to the expanded section.
- Behaviour, tab order and the lazy creation of the form stay as they are.

**Tests.** The arrow follows the checked state; existing insert tests keep passing.

**Docs.** `how-to/write-expressions.md`: describe clicking **Insert data point** to expand and collapse the form.

---

## Step 5 — Bounded help, shown with a disclosure control

**Finding.** The help appears as an unbounded block below the composing area. Long adapter help grows the dialog and can push the editor and the dialog buttons out of view on short displays. **Show info** has the same command-versus-disclosure problem as step 4.

Evidence: `expressionsdialog.ui` (`widgetInfo` in the main layout, `lblInfo` word-wrapped without scroll area, checkable `btnInfo` "Show info").

**Intent.** Help is available on demand without taking over the dialog; the editor and Accept/Cancel stay reachable.

**Proposed change.**

- Put `lblInfo` in a `QScrollArea` with a maximum height of about 15 text lines; longer help scrolls inside it. The device selector stays above it.
- `btnInfo` becomes a disclosure `QToolButton` like step 4, labelled **Expression help**, with the same arrow behaviour.
- The scroll area gets a place in the tab order after the device selector, so the help can be scrolled with the keyboard.

**Tests.** The help label is inside the scroll area and its height is bounded; the arrow follows the checked state; existing help tests keep passing.

**Docs.** `how-to/write-expressions.md`: one sentence on **Expression help**.

---

## Step 6 — Sidebar organised into Signal, Data points and Expression

**Finding.** The Signals panel stacks name, data-point list, address fields, expression preview, axis, status and actions in one flat column. When a signal combines several data points, it is not clear which fields affect the selected data point and which affect the whole signal.

Evidence: `signaleditorpanel.ui` (flat `QVBoxLayout`: `lineName`, `lblDataPoints`, `listDataPoints`, `addressContainer`, `lblExpression`, `axisGroupBox`, `lblStatus`, `btnAdd`, `btnEditExpression`).

**Intent.** Group the fields by what they affect, and make the selected data point clear.

**Proposed change.**

- **Signal** section: name, and the Y1/Y2 axis choice on one row (replaces the separate *Y-axis* group box).
- **Data point(s)** section: the data-point list (only for multi-point signals) and the address editor. The title reads *Data point*, or *Data points* when the signal combines several; the hint reads "This signal combines N data points. Select one to edit it."
- **Expression** section: the read-only expression preview (with the edited data point in bold) and **Edit expression…**. The redundant "Expression:" caption in the preview is dropped. In add mode the section is hidden until a reference has been built.
- Status line and the add button stay below the sections. Tab order follows the visual order.

**Risk — investigate before implementing.** A first attempt at this layout showed two problems that must be understood and fixed in this step:

1. With the fields inside group boxes, Enter in the name field no longer added the signal in the Wayland test run (`enterAddsSignal`, `enterInPanelDoesNotCloseDialog`). The key press reached the group box but not the panel. With the offscreen platform these tests passed. Expect the same in the real dialog until it is proven otherwise.
2. In a panel that is never shown, a group box lays out its children at its default size, which gave **Edit expression…** a height of 0 and made `editExpressionRequestsDialogForRow` miss its click.

Separately, two Enter tests already fail on the unchanged code under Wayland (`enterInAddressFieldAddsSignal`, `enterInInsertFormInserts`); they pass with `QT_QPA_PLATFORM=offscreen`. That is a pre-existing test-environment issue and not part of this step.

**Tests.** Section titles for single and multiple data points; Expression section visibility in add and edit mode; Enter still adds the signal; Edit expression still requests the dialog. Existing tests that compare the preview text are updated for the dropped caption.

**Docs.** `how-to/add-data-points.md`: refer to the Signal, Data point(s) and Expression sections in the add and edit steps.
