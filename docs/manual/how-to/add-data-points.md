# Add signals

This guide shows how to add signals to monitor in ModbusScope.

## Before you start

You must have at least one connection and one device configured. See [Configure a TCP connection](configure-tcp-connection.md) or [Configure an RTU connection](configure-rtu-connection.md).

## Steps

1. Click **Signals** in the toolbar.

   ![Signals dialog](<../_static/user_manual/add_register_dialog.png>)

2. Click **Add** to insert a new signal row.
3. In the **Expression** column, enter the data point address using `${...}` syntax (e.g. `${40001}`). See [Reference: Register syntax](../reference/register-syntax.md) for all supported forms.
4. In the **Name** column, enter a label for the graph legend.
5. In the **Color** column, pick a line color.
6. In the **Y-Axis** column, select `Y1` or `Y2`.
7. Use the **Active** checkbox to include or exclude a signal from polling.
8. Repeat steps 2–7 for each signal.
9. Click **OK**.

The data type is part of the expression (for example `${40001: s16b}`); see the register syntax reference.

**Result:** The signals appear in the main window. They will be polled when you click **Start logging**.

## Tips

- To reference a specific device, add `@DEVICE` to the expression (e.g. `${40001@2}` reads from device 2). Device 1 is used by default.
- To calculate or combine values, use an expression instead of a bare address. See [Write expressions](write-expressions.md).
- Importing registers from a `.mbc` file is faster than adding them by hand. See [Import from MBC file](import-mbc-file.md).
- Consecutive data point addresses are read in a single Modbus packet. Gaps between addresses split the request and slow down the poll rate. See [Optimize poll rate](optimize-poll-rate.md).

## See also

- [Reference: Register syntax](../reference/register-syntax.md)
