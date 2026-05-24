# Save and restore a project

This guide shows how to save your connection, device, and register configuration so you can reload it later.

## Save a project

1. Go to **File > Save Project As...**.
2. Choose a folder and enter a file name.
3. Click **Save**.

**Result:** A `.mbs` file is written containing all connection, device, and register settings. Logged data is not included.

## Open a saved project

**Option A — drag and drop:**

Drag the `.mbs` file from your file explorer onto the ModbusScope main window.

**Option B — menu:**

1. Go to **File > Open Project...**.
2. Select the `.mbs` file.

**Result:** All registers are restored. You can start logging immediately without reconfiguring anything.

## Notes

- You can set ModbusScope as the default application for `.mbs` files during installation, so double-clicking a project file opens it directly.
- Logged data is stored separately as a `.csv` file. See [Export logged data](export-data.md).

### Compatibility

Project files are now saved as JSON. `.mbs` files from ModbusScope v3 and v4 (XML format) load automatically and are saved as JSON the next time you use **File > Save Project As...**. Files saved before v3.x are not supported; open them in v3.x first and re-save.
