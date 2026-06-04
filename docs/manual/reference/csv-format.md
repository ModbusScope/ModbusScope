# CSV format reference

This page describes the format of `.csv` files written and read by ModbusScope.

## File structure

```text
Time (ms);Register 1;Register 2;Register 3
0;100;200;300
250;101;201;301
500;102;202;302
```

- **Row 1**: column headers. The first column is always the timestamp header; subsequent columns are register names.
- **Rows 2+**: one data row per poll cycle.

## Column layout

| Column | Content | Type |
| --- | --- | --- |
| 1 | Timestamp | Integer (ms from session start) or absolute date-time when absolute timestamps are enabled |
| 2..N | Register values | Numeric, one column per register in the order they appear in Registers |

## Separators (export)

ModbusScope uses system locale settings when writing. Typical combinations:

| Locale | Field separator | Decimal separator |
| --- | --- | --- |
| English | `,` | `.` |
| Belgian / French / German | `;` | `,` |

## Separators (import)

ModbusScope attempts to detect the separators automatically when opening a file. If detection fails, set them manually in the import dialog. See [Open a CSV data file](../how-to/open-csv-file.md).

| Setting | Meaning |
| --- | --- |
| Field separator | Character between columns |
| Decimal separator | Character between integer and fractional part |
| Thousand separator | Grouping character (may be absent) |

## Timestamp units (import)

| Setting | Behavior |
| --- | --- |
| Milliseconds (default) | Timestamp column read as-is |
| Seconds | Timestamp values multiplied by 1000 on load |

## See also

- [Export logged data (how-to)](../how-to/export-data.md)
- [Open a CSV data file (how-to)](../how-to/open-csv-file.md)
- [Preset file format](preset-file-format.md)
