# How expressions work

ModbusScope lets you attach an expression to each register row. Instead of displaying a raw register value directly, the application evaluates the expression and plots the result. This page explains what the expression engine can and cannot do.

## Two-stage evaluation

When ModbusScope reads a register, the process has two stages.

In the first stage, the application resolves all `${...}` references in the expression. Each reference is replaced with the numeric value read from the corresponding Modbus address. If any referenced address fails to read (timeout, Modbus exception, out-of-range index), the entire expression result is marked invalid for that cycle and no point is plotted.

In the second stage, the resulting arithmetic string is passed to the expression parser, which evaluates it to a single number. This value is what appears on the graph.

## What the parser can do

The parser supports the operators listed in [Expression operators reference](../reference/expression-operators.md): arithmetic, bitwise operations, shifts, and comparison/logical operators. It understands decimal, hexadecimal, and binary number literals, and it handles both `.` and `,` as decimal separators depending on which one appears first in the expression.

An expression can reference multiple registers. `${40001} * ${40002@2}` reads one register from device 1 and one from device 2, then multiplies them. All referenced registers are read in the same poll cycle.

## What the parser cannot do

There are no math functions (`sin`, `cos`, `sqrt`, and so on).

There is no state between samples. Each poll cycle evaluates the expression from scratch. You cannot compute a running average, accumulate a counter, or compare the current value to the previous one.

There are no string operations. All values are floating-point numbers.

## The compose expression dialog

The compose expression window evaluates the expression in real time as you type. The expression field turns green when the syntax is valid. The example input table lets you set hypothetical register values to verify that the expression produces the expected output before you commit it. This is useful for catching arithmetic mistakes without having to connect to a live device.

## See also

- [Write expressions (how-to)](../how-to/write-expressions.md)
- [Reference: Expression operators](../reference/expression-operators.md)
- [Reference: Register syntax](../reference/register-syntax.md)
