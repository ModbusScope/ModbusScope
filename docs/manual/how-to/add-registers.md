# Add Modbus registers

This guide shows how to add registers to monitor in ModbusScope.

## Before you start

You must have at least one connection and one device configured. See [Configure a TCP connection](configure-tcp-connection.md) or [Configure an RTU connection](configure-rtu-connection.md).

## Steps

1. Click **Register Settings** in the toolbar.

   ![Register settings dialog](<../_static/user_manual/add_register_dialog.png>)

2. Click **Add** to insert a new register row.
3. In the **Expression** column, enter the register address using `${...}` syntax (e.g. `${40001}`). See [Reference: Register syntax](../reference/register-syntax.md) for all supported forms.
4. In the **Name** column, enter a label for the graph legend.
5. In the **Color** column, pick a line color.
6. In the **Data type** column, select the type that matches your device (`16b`, `s16b`, `32b`, `s32b`, `f32b`). For coils and discrete inputs, the type is ignored.
7. In the **Y-axis** column, select `Y1` or `Y2`.
8. Repeat steps 2–7 for each register.
9. Click **OK**.

**Result:** The registers appear in the main window. They will be polled when you click **Start Logging**.

## Tips

- To reference a specific device, add `@DEVICE` to the expression (e.g. `${40001@2}` reads from device 2). Device 1 is used by default.
- To calculate or combine values, use an expression instead of a bare address. See [Write expressions](write-expressions.md).
- Importing registers from a `.mbc` file is faster than adding them by hand. See [Import from MBC file](import-mbc-file.md).
- Consecutive register addresses are read in a single Modbus packet. Gaps between addresses split the request and slow down the poll rate. See [Optimize poll rate](optimize-poll-rate.md).

## See also

- [Reference: Register syntax](../reference/register-syntax.md)
