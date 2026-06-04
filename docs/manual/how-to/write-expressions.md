# Write a register expression

This guide shows how to write an expression that calculates or combines register values.

## Steps

1. Click **Registers** in the toolbar.
2. In the row you want to edit, click the `...` button in the **Expression** column to open the **Compose expression** window.

   ![Compose expression dialog](<../_static/user_manual/expression_dialog.png>)

3. Type your expression in the input field. You can reference registers with `${ADDRESS}` and combine them using operators.

   Examples:
   - Convert a raw value to amps: `${40001: s16b} * 0.1`
   - Combine voltage and current into power: `${40001} * ${40002@2}`
   - Extract the high byte of a register: `(${40001} >> 8) & 0xFF`

4. Use the **Example input** table to enter test values and verify the result. The output updates as you type.
5. When the expression field turns green, the syntax is valid.
6. Click **OK** to apply.

**Result:** The register row uses the expression. The graph shows the calculated value instead of the raw register value.

## Error handling

If the expression or a test input produces an invalid result (such as division by zero or an invalid register read), the output field shows an error message instead of a value. Correct the expression before applying.

## See also

- [Reference: Register syntax](../reference/register-syntax.md)
- [Reference: Expression operators](../reference/expression-operators.md)
- [Explanation: How expressions work](../explanation/expressions.md)
