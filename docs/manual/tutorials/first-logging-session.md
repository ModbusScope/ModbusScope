# Your first logging session

This tutorial walks you through a complete ModbusScope session from a fresh install to a saved CSV file. You will connect to a Modbus TCP device, add three registers, log data, and export it.

**What you need:**

- ModbusScope installed on your computer ([download](https://github.com/ModbusScope/ModbusScope/releases/latest))
- A Modbus TCP device reachable on your network (this tutorial uses `192.168.1.10`, port `502`, slave ID `1`)
- Three holding registers at addresses `40001`, `40002`, `40003`

If your device uses different values, substitute them at each step.

---

## Step 1 — Add a connection

1. Open ModbusScope.
2. Go to **Settings > Connection**.
3. Set **Protocol** to `TCP`.
4. Set **IP address** to `192.168.1.10`.
5. Set **Port** to `502`.
6. Leave **Timeout** at the default.
7. Click **OK**.

## Step 2 — Add a device

1. Go to **Settings > Device**.
2. Click **Add device**.
3. Set **Name** to `My device`.
4. Set **Connection ID** to `1` (the connection you just created).
5. Set **Slave ID** to `1` (the Modbus slave ID of the device).
6. Click **OK**.

## Step 3 — Add registers

1. Click **Register Settings** in the toolbar.
2. Click **Add** to add a new row.
3. In the **Expression** column, type `${40001}`. Set **Name** to `Register 1`.
4. Add a second row: expression `${40002}`, name `Register 2`.
5. Add a third row: expression `${40003}`, name `Register 3`.
6. Click **OK**.

## Step 4 — Start logging

1. Click **Start Logging**.

The graph starts updating in real time. Each register appears as a line.

You can pan and zoom the live graph at any time: scroll to zoom, drag to pan. This is useful for inspecting recent samples while logging continues.

> If the graph shows no data or the status bar shows connection errors, verify the IP address, port, and slave ID in Steps 1 and 2.

## Step 5 — Navigate the graph

The **Scale options** panel at the bottom of the window controls how the axes scale.

1. Under **X axis**, select **Sliding window** and set the interval to `60` seconds.

   The graph now shows only the last 60 seconds of data, scrolling forward as new samples arrive.

2. Under **Y axis**, select **Window auto-scale**.

   The y-axis rescales automatically to the values visible in the current x-window.

You can also navigate with the mouse: scroll to zoom both axes, drag to pan. To zoom a single axis, click its label first, then scroll or drag. Double-click an axis label to reset it to auto-scale.

## Step 6 — Stop logging

1. Click **Stop Logging** when you have collected enough data.

The graph stops updating. You can continue to pan and zoom the captured data before exporting it.

## Step 7 — Export to CSV

1. Go to **File > Save Data File As...**.
2. Choose a folder and file name.
3. Click **Save**.

**Result:** You have a `.csv` file containing timestamps and the values of all three registers, one column each.

---

## What's next

- [Navigate the graph: scaling, zoom, and pan](../how-to/navigate-the-graph.md)
- [Add a second connection and device](../how-to/configure-tcp-connection.md)
- [Write expressions to convert or combine register values](../how-to/write-expressions.md)
- [Save your register configuration as a project file](../how-to/save-restore-project.md)
