# Expression operators reference

Register references (`${...}`) are resolved first; the resulting numeric values are then passed to the expression parser.

## Arithmetic operators

| Operator | Operation | Example |
|---|---|---|
| `+` | Addition | `${40001} + 100` |
| `-` | Subtraction | `${40001} - ${40002}` |
| `*` | Multiplication | `${40001} * 0.1` |
| `/` | Division | `${40001} / 4` |
| `^` | Exponentiation | `${40001} ^ 2` |
| `%` | Modulus (integers only) | `${40001} % 256` |

## Bitwise operators

Operands are truncated to 32-bit integers before the operation.

| Operator | Operation | Example |
|---|---|---|
| `&` | Bitwise AND | `${40001} & 0xFF` |
| `\|` | Bitwise OR | `${40001} \| 0x8000` |
| `>>` | Right shift | `${40001} >> 8` |
| `<<` | Left shift | `${40001} << 2` |

## Comparison operators

Return `1.0` (true) or `0.0` (false).

| Operator | Meaning |
|---|---|
| `==` | Equal |
| `!=` | Not equal |
| `<` | Less than |
| `>` | Greater than |
| `<=` | Less than or equal |
| `>=` | Greater than or equal |

## Logical operators

| Operator | Meaning | Notes |
|---|---|---|
| `&&` | Logical AND | Returns `1.0` or `0.0` |
| `\|\|` | Logical OR | Returns `1.0` or `0.0` |

## Operator precedence (high to low)

1. `^`
2. `*`, `/`, `%`
3. `>>`, `<<`
4. `+`, `-`
5. `&`
6. `|`
7. `<`, `>`, `<=`, `>=`, `==`, `!=`
8. `&&`
9. `||`
10. `?:` (ternary)

Use parentheses to override precedence.

## Unary operators

| Operator | Meaning |
|---|---|
| `-` | Negate |
| `+` | Identity (no effect) |

## Number literals

| Format | Prefix | Example |
|---|---|---|
| Decimal | none | `42`, `3.14`, `1,5` |
| Hexadecimal | `0x` | `0xFF`, `0x1000` |
| Binary | `0b` | `0b11111000` |

**Decimal separator:** ModbusScope uses the first separator it encounters — `.` or `,` — for the entire expression. Mixing both in one expression is invalid.

**Function argument separator:** `;` (regardless of locale).

## Limitations

- No math functions (`sin`, `cos`, `sqrt`, etc.) are available.
- No string operations.
- No state between samples: each expression is evaluated independently per poll cycle.
- No access to previous sample values.

## See also

- [Register syntax](register-syntax.md)
- [Write expressions (how-to)](../how-to/write-expressions.md)
- [Explanation: How expressions work](../explanation/expressions.md)
