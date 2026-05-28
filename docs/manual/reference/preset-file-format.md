# Preset file format reference

Presets are stored in a JSON file named `presets.json`. See [Create CSV presets](../how-to/create-csv-presets.md) for where to place the file.

## Top-level structure

```json
{
  "presets": [
    { ... },
    { ... }
  ]
}
```

The file contains a single `presets` array. Each object in the array is one preset.

## Preset fields

| Field | Required | Type | Description |
| --- | --- | --- | --- |
| `name` | Yes | String | Display name shown in the preset dropdown |
| `fieldseparator` | Yes | String | Character separating columns. Use `\t` for tab |
| `decimalseparator` | Yes | String | Character for decimal point |
| `keyword` | No | String | If the opened filename contains this string (case-insensitive), this preset is selected automatically |
| `thousandseparator` | No | String | Grouping character for large numbers |
| `commentsequence` | No | String (1–2 chars) | Lines beginning with this string are skipped |
| `column` | No | Integer (1-based) | Column index of the timestamp. Default: `1` |
| `labelrow` | No | Integer (1-based) | Row containing column headers. Use `-1` for files with no header. Default: `1` |
| `datarow` | No | Integer (1-based) | First row of data. Default: `1` |
| `timeinmilliseconds` | No | Boolean | Default `true`. Set to `false` to convert timestamps from seconds to milliseconds on load |

## Example

```json
{
  "presets": [
    {
      "name": "Default (BE)",
      "keyword": "-be",
      "fieldseparator": ";",
      "decimalseparator": ",",
      "thousandseparator": " ",
      "commentsequence": "//",
      "column": 1,
      "labelrow": 1,
      "datarow": 2
    },
    {
      "name": "Seconds timestamps",
      "fieldseparator": ";",
      "decimalseparator": ",",
      "thousandseparator": " ",
      "commentsequence": "//",
      "column": 1,
      "labelrow": 1,
      "datarow": 2,
      "timeinmilliseconds": false
    }
  ]
}
```

## Notes

- If a valid preset file is found, it replaces the built-in presets entirely.
- Only one file is used: the first valid file found in the search order (user Documents folder, then executable folder).

## Backward compatibility — XML format

ModbusScope still loads the older `presets.xml` format. If you have an existing `presets.xml`, it continues to work without any changes. The format is detected automatically.

The search order is: `presets.json` (preferred) → `presets.xml` in the Documents folder, then the same pair in the executable folder. If you create a new `presets.json`, it takes priority over any `presets.xml` in the same location.

## See also

- [Create CSV presets (how-to)](../how-to/create-csv-presets.md)
- [CSV format](csv-format.md)
