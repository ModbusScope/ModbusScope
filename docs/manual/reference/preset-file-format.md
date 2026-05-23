# Preset file format reference

Presets are stored in an XML file named `presets.xml`. See [Create CSV presets](../how-to/create-csv-presets.md) for where to place the file.

## Top-level structure

```xml
<modbusscope>
  <parsepreset>...</parsepreset>
  <parsepreset>...</parsepreset>
</modbusscope>
```

The file may contain any number of `<parsepreset>` elements.

## `<parsepreset>` elements

| Element | Required | Type | Description |
|---|---|---|---|
| `<name>` | Yes | String | Display name shown in the preset dropdown |
| `<keyword>` | No | String | If the opened filename contains this string (case-sensitive), this preset is selected automatically |
| `<fieldseparator>` | Yes | Single char | Character separating columns |
| `<decimalseparator>` | Yes | Single char | Character for decimal point |
| `<thousandseparator>` | No | Single char | Grouping character for large numbers |
| `<commentSequence>` | No | 1–2 chars | Lines beginning with this string are skipped |
| `<column>` | Yes | Integer (1-based) | Column index of the timestamp |
| `<labelrow>` | Yes | Integer (1-based) | Row containing column headers |
| `<datarow>` | Yes | Integer (1-based) | First row of data |
| `<timeinmilliseconds>` | No | `true` / `false` | Default `true`. Set to `false` to convert the timestamp from seconds to milliseconds on load |

## Example

```xml
<modbusscope>
  <parsepreset>
    <name>Default (BE)</name>
    <keyword>-be</keyword>
    <fieldseparator><![CDATA[;]]></fieldseparator>
    <decimalseparator><![CDATA[,]]></decimalseparator>
    <thousandseparator><![CDATA[ ]]></thousandseparator>
    <commentSequence><![CDATA[//]]></commentSequence>
    <column>1</column>
    <labelrow>1</labelrow>
    <datarow>2</datarow>
  </parsepreset>

  <parsepreset>
    <name>Seconds timestamps</name>
    <fieldseparator><![CDATA[;]]></fieldseparator>
    <decimalseparator><![CDATA[,]]></decimalseparator>
    <thousandseparator><![CDATA[ ]]></thousandseparator>
    <commentSequence><![CDATA[//]]></commentSequence>
    <column>1</column>
    <labelrow>1</labelrow>
    <datarow>2</datarow>
    <timeinmilliseconds>false</timeinmilliseconds>
  </parsepreset>
</modbusscope>
```

## Notes

- Use `<![CDATA[...]]>` wrappers for separator characters to avoid XML escaping issues.
- If a valid `presets.xml` is found, it replaces the built-in presets entirely.
- Only one file is used: the first valid file found in the search order (user Documents folder, then executable folder).

## See also

- [Create CSV presets (how-to)](../how-to/create-csv-presets.md)
- [CSV format](csv-format.md)
