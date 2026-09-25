# CSV format reference

This page describes the format of `.csv` files written and read by ModbusScope.

## File structure

```text
Time (ms);Signal 1;Signal 2;Signal 3
0;100;200;300
250;101;201;301
500;102;202;302
```

- **Row 1**: column headers. The first column is always the timestamp header; subsequent columns are signal names.
- **Rows 2+**: one data row per poll cycle.

## Column layout

| Column | Content | Type |
| --- | --- | --- |
| 1 | Timestamp | Integer (ms from session start) or absolute date-time when absolute timestamps are enabled |
| 2..N | Signal values | Numeric, one column per signal in the order they appear in Signals |

## Data quality columns

Files written by ModbusScope contain a `//Quality;1` header line. It means every signal column is directly followed by a quality column for that signal:

```text
//Quality;1
Time (ms);Temp;Temp (quality);Press;Press (quality)
0;21.5;0;1.02;0
1000;21.6;129;1.02;0
2000;0;2;1.03;0
```

The quality code is `state + flags`:

| Code | Meaning |
| --- | --- |
| 0 | Good |
| 1 | Degraded |
| 2 | Invalid (value written as 0) |
| 3 | No value (value written as 0) |
| +16 | Substituted |
| +32 | Blocked |
| +64 | Overflow |
| +128 | Old data |

For example, `129` is Degraded with Old data. On import, an empty quality cell reads as Good, unknown flag bits are ignored and an unknown state reads as Invalid. Files without the `//Quality` line load with every sample marked Good.

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
