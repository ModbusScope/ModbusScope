# Graph scaling and navigation

ModbusScope offers several axis scaling modes to support two distinct workflows: **live monitoring** while data is being logged, and **post-session analysis** after logging has stopped. Understanding the difference helps you choose the right mode for each task.

## X-axis: following live data vs. reviewing history

The default **Full auto-scale** mode keeps the entire logged time range in view — useful for a broad overview but increasingly crowded as a session grows. **Sliding window** mode instead anchors the right edge of the graph to the most recent sample and shows only the last N seconds. This is the natural choice during active logging: the graph scrolls forward with each new sample, giving a live oscilloscope-like view without the x-axis continuously rescaling.

**Manual** mode is entered automatically when you drag the graph or draw a zoom rectangle. It freezes the x-range at whatever you have navigated to, so you can examine a specific period without the view jumping. Double-clicking the x-axis label returns it to its previous auto-scale mode.

## Y-axis: tracking the visible window vs. all history

**Full auto-scale** sizes the y-axis to all values ever logged, which can make recent detail hard to see if earlier data had a very different range. **Window auto-scale** restricts the y-axis to the values present in the currently visible x-window — as you pan and zoom the x-axis the y-axis rescales to match. Pairing **Sliding window** on the x-axis with **Window auto-scale** on the y-axis is the recommended setup for live monitoring: both axes stay focused on what is happening right now.

**Limit from / to** fixes the y-axis at specific bounds, useful when you know the expected operating range and want consistent vertical scale across sessions. **Manual** mode, as with the x-axis, is entered automatically on drag or zoom-rectangle operations and can be reset by double-clicking the axis label.

## See also

- [Navigate the graph (how-to)](../how-to/navigate-the-graph.md)
- [Reference: Graph scaling](../reference/graph-scaling.md)
