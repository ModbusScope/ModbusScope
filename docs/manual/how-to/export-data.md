# Export logged data

This guide shows how to export data from a logging session as a CSV file or as an image.

> You cannot export while logging is active. Stop logging first.

## Export as CSV

1. Go to **File > Save Data File As...**.
2. Choose a folder and enter a file name.
3. Click **Save**.

**Result:** A `.csv` file is written with one column for the timestamp and one column per register.

## Export as image

1. Go to **File > Export Image As...**.
2. Choose a folder, enter a file name, and select an image format.
3. Click **Save**.

**Result:** An image of the current graph view is saved to disk.

## Notes

- The CSV format uses the field and decimal separators configured in your locale settings.
- To re-open an exported CSV in ModbusScope, see [Open a CSV data file](open-csv-file.md).
- The image export captures the current zoom level and visible area of the graph.

## See also

- [Reference: CSV format](../reference/csv-format.md)
- [Open a CSV data file](open-csv-file.md)
