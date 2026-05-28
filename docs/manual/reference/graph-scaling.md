# Graph scaling reference

## X-axis scaling modes

| Mode | Behaviour |
| ------ | ----------- |
| **Full auto-scale** | The x-axis always spans the full extent of all logged data. |
| **Sliding window** | The x-axis shows a fixed-width time window ending at the most recent sample. The window width is set in seconds (1–9999). |
| **Manual** | The x-axis range is controlled entirely by drag and scroll. ModbusScope sets this mode automatically when you drag or use the zoom rectangle. |

## Y-axis and Y2-axis scaling modes

| Mode | Behaviour |
| ------ | ----------- |
| **Full auto-scale** | The y-axis spans all data values across the entire logged time range, even if some are outside the visible x-window. |
| **Window auto-scale** | The y-axis spans only the values visible in the current x-window. As you pan or zoom the x-axis, the y-axis rescales to fit. |
| **Limit from / to** | The y-axis is fixed at a user-defined minimum and maximum. Enter the values in the Scale options panel. |
| **Manual** | The y-axis range is controlled entirely by drag and scroll. Set automatically when you drag the y-axis or use the zoom rectangle. |

## Mouse interactions

| Action | Result |
| -------- | -------- |
| Scroll wheel (no axis selected) | Zoom both axes simultaneously, centred on the cursor |
| Click an axis label, then scroll | Zoom that axis only |
| Drag (no axis selected) | Pan both axes |
| Click an axis label, then drag | Pan that axis only |
| Double-click an axis label | Reset that axis to auto-scale |
| **Zoom** toolbar button, then drag a rectangle | Zoom into the selected area; all affected axes switch to Manual |

## See also

- [Navigate the graph (how-to)](../how-to/navigate-the-graph.md)
- [Graph scaling and navigation (explanation)](../explanation/graph-scaling.md)
