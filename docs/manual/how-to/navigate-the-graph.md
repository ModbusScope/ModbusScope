# Navigate the graph

This guide shows how to use the axis scaling modes and mouse interactions to monitor live data and examine recorded samples.

The **Scale options** panel at the bottom of the main window controls how each axis scales. You can change modes at any time, including while logging is active.

## Set up for live monitoring

The most useful setup for watching data in real time is a sliding x-axis combined with a y-axis that tracks the visible window.

1. In the **Scale options** panel, under **X axis**, select **Sliding window**.
2. Set the interval (in seconds) to the time range you want visible — for example `60` for the last minute.
3. Under **Y axis**, select **Window auto-scale**.

The graph now scrolls forward as new samples arrive, and the y-axis rescales to the values visible in the current window.

## Zoom with the scroll wheel

- **Both axes**: scroll the mouse wheel anywhere on the graph (with no axis label selected).
- **One axis only**: click an axis label to select it, then scroll.

## Pan by dragging

- **Both axes**: drag anywhere on the graph (no axis label selected).
- **One axis only**: click an axis label to select it, then drag.

## Zoom into a specific region

1. Click the **Zoom** button in the toolbar.
2. Drag a rectangle over the area of interest on the graph.

The graph zooms into the selected region and all affected axes switch to **Manual** mode.

## Reset an axis to auto-scale

Double-click the axis label (the text along the axis edge). The axis returns to its previous auto-scale mode.

## Set a fixed y-axis range

1. In the **Scale options** panel, under **Y axis**, select **Limit from**.
2. Enter the minimum and maximum values.

The y-axis is held at these bounds regardless of the logged values.

## See also

- [Reference: Graph scaling](../reference/graph-scaling.md)
- [Explanation: Graph scaling and navigation](../explanation/graph-scaling.md)
- [Measure with markers](use-markers.md)
