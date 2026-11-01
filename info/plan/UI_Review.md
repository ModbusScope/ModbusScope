# UI Review: Signals and Compose Expression

## Overall assessment

The dialogs have a solid functional foundation, but their visual hierarchy is still fairly utilitarian. The Signals dialog is a dense table-and-sidebar workspace. Compose expression is a technical form where composing, inserting data points, testing sample values, and reading help compete for attention.

The biggest UI opportunities are to make the primary task more prominent, contain optional content, and clarify which controls are actions versus expandable sections.

## Highest-impact findings

| Priority | Finding | Why it matters | Recommendation |
|---|---|---|---|
| **High** | **The insert control looks like a command but behaves like a disclosure.** “Insert data point…” conventionally suggests opening another dialog, but this control expands an inline form and stays checked while open. | Users may expect a separate picker, miss how to collapse the form, or be surprised when the dialog grows. | Give the control explicit disclosure treatment, such as **Insert data point** with a chevron and clear expanded/collapsed state. Keep the expression editor visually dominant. |
| **High** | **Optional help can overwhelm the composing workspace.** The help appears as a large block below the main form. | When expanded, it competes with expression editing and sample testing for vertical space and can push the primary task out of view on shorter displays. | Put help in a bounded scroll area or side panel, or show concise essentials with a separate “More…” expansion. Keep the editor and dialog actions accessible. |
| **Medium** | **The expression area has redundant framing and inflexible sizing.** The window title and “Compose Expression” group-box title repeat the same idea. The editor has a fixed three-line height. | Repeated headings consume space, and a fixed editor can feel cramped for long formulas or oversized for short ones. | Remove one title layer and let the editor resize within sensible minimum and maximum bounds. Retain wrapping and scrolling for long expressions. |
| **Medium** | **Sample-testing labels do not clearly explain the relationship between inputs and output.** “Example Input” is a section heading, the editable column is “Value,” and “Processed Output” sits separately. | Users may mistake example values for live readings or miss that changing them recalculates the result. | Use labels such as **Test values**, **Input value**, and **Calculated result**; group the output visually with the sample inputs. |
| **Medium** | **Signals has two add affordances with subtly different meanings.** The dialog-level **New signal** button enters add mode, while the panel button **Add signal** commits the current form. | The distinction is valid, but similar labels make it harder to predict which one starts the workflow and which one adds a row. | Use one clear entry-point label, such as **Add signal**, and a distinct commit label such as **Add to list**. Keep **New signal** as the panel's mode heading. |
| **Medium** | **The sidebar's information hierarchy is implicit.** Name, data-point selection, address fields, expression preview, axis, status, and actions are stacked together. | When editing a multi-point formula, users must infer which fields affect the selected data point and which affect the whole signal. | Organize the panel into clear sections: **Signal** (name and axis), **Data points** (selection and address editor), and **Expression** (read-only preview and edit action). Make the selected data point especially clear. |

## Strengths to preserve

- The Signals dialog keeps the table and editor panel visible together, avoiding a context-switching popup workflow.
- The expression preview connects structured data-point fields to the resulting formula.
- Compose expression separates formula editing from example evaluation and wraps long expressions.
- Cancel and the Signals dialog's draft behavior give users a way to discard the broader set of edits.

## Visual-evidence caveat

The manual screenshot at [`docs/manual/_static/user_manual/expression_dialog.png`](../../docs/manual/_static/user_manual/expression_dialog.png) appears not to match the current UI definition and behavior. For example, it shows a much taller expression editor and different action placement. Refresh the screenshot before using it as a reference for visual QA.

This review focuses on visual hierarchy, layout, labeling, and presentation; it is not a live usability session.





