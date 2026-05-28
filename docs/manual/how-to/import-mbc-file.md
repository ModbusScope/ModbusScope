# Import registers from an MBC file

This guide shows how to import register definitions from a ModbusControl (`.mbc`) project file.

## Steps

**Option A — drag and drop:**

1. Drag the `.mbc` file from your file explorer onto the ModbusScope main window.
2. The import dialog opens automatically.

**Option B — menu:**

1. Click **Register Settings** in the toolbar.
2. Click **Import from .mbc file**.
3. Select your `.mbc` file.

After opening the file with either option:

1. In the import dialog, review the registers found in the file.

   ![MBC import dialog](<../_static/user_manual/import_from_mbc_dialog.png>)

2. Select the registers you want to add and click **Import**.

**Result:** The selected registers are added to the register list with their names and addresses. You can edit them further in the Register Settings dialog.

## Notes

- ModbusControl is a separate proprietary application. This import feature only reads register definitions; it does not communicate with ModbusControl.
- Imported registers use default types and colors. Adjust them in Register Settings if needed.

## See also

- [Add registers manually](add-registers.md)
