# Add signals

This guide shows how to add signals to monitor in ModbusScope.

## Before you start

You must have at least one connection and one device configured. See [Configure a TCP connection](configure-tcp-connection.md) or [Configure an RTU connection](configure-rtu-connection.md).

## Steps

1. Click **Signals** in the toolbar.

   ![Signals dialog](<../_static/user_manual/add_register_dialog.png>)

2. Click the arrow next to **Add** to open the add form.
3. Select the **device**. The line below it shows the device's protocol.
4. Optionally enter a **name** for the graph legend. A signal without a name is called *New signal*, *New signal 2*, and so on.
5. Fill in the address fields. They depend on the device's protocol, for example the object type, address and data type for Modbus.
6. Select **Y1 axis** or **Y2 axis**.
7. Click **Add signal**. The signal is added to the table and the form closes.

   If the adapter rejects the address, the form stays open and shows the reason above the **Add signal** button. Correct the fields and click **Add signal** again.

8. Repeat steps 2–7 for each signal.
9. In the table, adjust the **Color** and the **Active** checkbox, or rename a signal in the **Name** column.
10. Click **OK**.

Clicking **Add** itself, instead of its arrow, inserts a row with a default expression that you can edit in the **Expression** column with `${...}` syntax. See [Reference: Register syntax](../reference/register-syntax.md).

**Result:** The signals appear in the main window. They will be polled when you click **Start logging**.

## Tips

- To reference a specific device, add `@DEVICE` to the expression (e.g. `${40001@2}` reads from device 2). Device 1 is used by default.
- To calculate or combine values, use an expression instead of a bare address. See [Write expressions](write-expressions.md).
- Importing registers from a `.mbc` file is faster than adding them by hand. See [Import from MBC file](import-mbc-file.md).
- Consecutive data point addresses are read in a single Modbus packet. Gaps between addresses split the request and slow down the poll rate. See [Optimize poll rate](optimize-poll-rate.md).

## See also

- [Reference: Register syntax](../reference/register-syntax.md)
