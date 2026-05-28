# Open a CSV data file

This guide shows how to load a CSV file into ModbusScope for viewing on the graph.

## Steps

**Option A — drag and drop:**

Drag the `.csv` file from your file explorer onto the ModbusScope main window.

**Option B — menu:**

1. Go to **File > Open Data File...**.
2. Select the `.csv` file.

After opening the file with either option:

1. ModbusScope attempts to detect the field and decimal separators automatically.

   ![CSV import dialog](<../_static/user_manual/import_csv.png>)

2. If the preview shows garbled data, adjust the separator settings manually:
   - **Field separator**: the character between columns (e.g. `;` or `,`)
   - **Decimal separator**: the character used for decimals (e.g. `.` or `,`)
   - **Thousand separator**: the grouping character (e.g. `.` or ` `)

3. Set **Timestamp unit** to `ms` or `s` to match the unit in your file. ModbusScope always stores timestamps in milliseconds internally.

4. If the file has header lines or the timestamp is not in the first column, adjust:
   - **Comment string**: lines starting with this string are skipped
   - **Timestamp column**: column index of the timestamp (1-based)
   - **Label row**: row number of the column headers
   - **Data row**: row number where data starts

5. Click **OK**.

**Result:** The data is displayed on the graph as if it were a live logging session.

## Notes

- CSV files exported by ModbusScope open without any manual configuration.
- For files you open repeatedly with non-standard settings, save a preset to avoid reconfiguring each time. See [Create CSV presets](create-csv-presets.md).

## See also

- [Reference: CSV format](../reference/csv-format.md)
- [Reference: Preset file format](../reference/preset-file-format.md)
- [Create CSV presets](create-csv-presets.md)
