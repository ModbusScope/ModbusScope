# Write an expression

This guide shows how to write an expression that calculates or combines data point values.

## Steps

1. Click **Signals** in the toolbar.
2. In the row you want to edit, click the `...` button in the **Expression** column to open the **Compose expression** window.

   ![Compose expression dialog](<../_static/user_manual/expression_dialog.png>)

3. Type your expression in the input field. You can reference data points with `${ADDRESS}` and combine them using operators.

   Examples:
   - Convert a raw value to amps: `${40001: s16b} * 0.1`
   - Combine voltage and current into power: `${40001} * ${40002@2}`
   - Extract the high byte of a data point: `(${40001} >> 8) & 0xFF`

4. Use the **Example Input** table to enter test values and verify the result. The output updates as you type. Values can be decimal, hexadecimal (`0x10`) or binary (`0b101`); every data point starts at `1`.
5. Click **Accept** or press **Enter** to apply.

**Accept** is unavailable while the expression has a syntax error or refers to a device that does not exist. The message below the expression says what is wrong.

**Result:** The signal uses the expression. The graph shows the calculated value instead of the raw data point value.

## Error handling

If a test input produces an invalid result (such as a division by zero), the output field shows `-` and the error is shown below the expression. This depends only on the test values, so you can still apply the expression.

## See also

- [Reference: Register syntax](../reference/register-syntax.md)
- [Reference: Expression operators](../reference/expression-operators.md)
- [Explanation: How expressions work](../explanation/expressions.md)
