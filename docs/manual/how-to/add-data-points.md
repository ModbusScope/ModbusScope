# Add signals

This guide shows how to add signals to monitor in ModbusScope.

## Before you start

You must have at least one connection and one device configured. See [Configure a TCP connection](configure-tcp-connection.md) or [Configure an RTU connection](configure-rtu-connection.md).

## Steps

1. Click **Signals** in the toolbar.

   ![Signals dialog](<../_static/user_manual/add_register_dialog.png>)

2. The panel on the right of the table adds signals. If a row is selected, click **New signal** to start a new signal.
3. Optionally enter a **name** for the graph legend. A signal without a name is called *New signal*, *New signal 2*, and so on.
4. Select the **device**. The line below it shows the device's protocol.
5. Fill in the address fields. They depend on the device's protocol, for example the object type, address and data type for Modbus. Below the fields, **Reference** shows the data point reference that will be added, for example `${h0}`. It is updated after each change.
6. Select **Y1 axis** or **Y2 axis**.
7. Click **Add signal** or press Enter. The signal is added to the table and the line above the button names it. The panel stays open and keeps the device, the address fields and the axis, so for the next signal you only change what differs, for example the address.

   If the adapter rejects the address, no reference is shown, **Add signal** is disabled and the reason appears above it. Correct the fields to try again.

8. Repeat steps 3–7 for each signal.
9. In the table, adjust the **Color** and the **Active** checkbox, or rename a signal in the **Name** column.

   The **Data point** column shows what each signal reads, as the device's adapter describes it, for example *Holding register 40001, unsigned 16-bit*. A signal that reads several data points shows their number; hover over it to see each one. Until the adapter has answered, or while it is not running, the column shows the data point reference. A reference the adapter rejects is shown in red, with the reason in its tooltip. For an expression with a syntax error, the column shows the error.
10. Click **OK** to apply the changes. **Cancel** discards every change made since the dialog opened.

    On **OK**, signals whose expression changed, and signals that you deactivated, lose the data of the previous log. Other signals keep their data.

To calculate with a data point, add the signal first and then click **…** in its **Expression** column. See [Write expressions](write-expressions.md) and [Reference: Register syntax](../reference/register-syntax.md).

**Result:** The signals appear in the main window. They will be polled when you click **Start logging**.

## Edit a signal

1. In the **Signals** dialog, select the signal in the table. The panel shows it under **Edit signal**.
2. Change the **name** or the axis. The table is updated while you type.
3. Change the device or the address fields of its data point. Each change replaces the data point reference in the expression, for example `${40001}` becomes `${h5}`. The rest of the expression is kept: `${40001}/2` becomes `${h5}/2`, and the data point being edited is shown in bold.

   Selecting a device on another protocol resets the address fields to that protocol's defaults.

4. A signal that reads several data points lists them, one entry for each time a data point occurs in the expression. Select an entry to edit that data point; the others stay as they are.
5. Click **Edit expression…** to change the expression itself. For an expression with a syntax error, the panel only shows the error and this button.
6. Click **OK** to apply the changes, or **Cancel** to discard them.

Until the adapter has described the data point, its fields cannot be edited. If the adapter is not running or rejects the data point, the reason is shown above **Edit expression…**; changing a field replaces the data point.

To add a new signal again, click **New signal** or clear the selection.

## Tips

- To reference a specific device, add `@DEVICE` to the expression (e.g. `${40001@2}` reads from device 2). Device 1 is used by default.
- To calculate or combine values, use an expression instead of a bare address. See [Write expressions](write-expressions.md).
- Importing registers from a `.mbc` file is faster than adding them by hand. See [Import from MBC file](import-mbc-file.md).
- Consecutive data point addresses are read in a single Modbus packet. Gaps between addresses split the request and slow down the poll rate. See [Optimize poll rate](optimize-poll-rate.md).

## See also

- [Reference: Register syntax](../reference/register-syntax.md)
