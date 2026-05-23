# Create CSV parse presets

This guide shows how to create a `presets.xml` file so that ModbusScope automatically applies the right parse settings when opening certain CSV files.

## Steps

1. Create a file named `presets.xml` and place it in one of these locations:
   - `C:\Users\<USER>\Documents\ModbusScope\presets.xml` (Windows, checked first)
   - Same folder as the ModbusScope executable (fallback)

2. Write your presets using this structure:

   ```xml
   <modbusscope>
     <parsepreset>
       <name>My preset</name>
       <keyword>-mydevice</keyword>
       <fieldseparator><![CDATA[;]]></fieldseparator>
       <decimalseparator><![CDATA[,]]></decimalseparator>
       <thousandseparator><![CDATA[ ]]></thousandseparator>
       <commentSequence><![CDATA[//]]></commentSequence>
       <column>1</column>
       <labelrow>1</labelrow>
       <datarow>2</datarow>
     </parsepreset>
   </modbusscope>
   ```

3. Adjust the values to match your file format. See [Reference: Preset file format](../reference/preset-file-format.md) for all available elements.

4. Save the file and reopen ModbusScope (or open a CSV file — presets are loaded when the import dialog opens).

**Result:** When you open a CSV file whose name contains the keyword (e.g. a file named `log-mydevice.csv`), the matching preset is selected automatically. If no keyword matches, you can pick a preset from the dropdown in the import dialog.

## Notes

- If a valid `presets.xml` is found, it replaces the built-in presets entirely.
- A preset can omit `<keyword>` if you always want to select it manually.
- Timestamps in seconds: add `<timeinmilliseconds>false</timeinmilliseconds>` to have ModbusScope convert them on load.

## See also

- [Reference: Preset file format](../reference/preset-file-format.md)
- [Open a CSV data file](open-csv-file.md)
