# Importing and exporting

## Storing and reusing configuration

The configuration of registers, as described in previous sections, can be saved and loaded in a project file. These settings can be saved in a `.mbs` file by going to *File > Save Project As...* and loaded by going to *File > Open Project...* or by dragging a `.mbs` file into the application.

## Exporting data/image

Current log results can be exported as an image or as data (`.csv`) file. You can select either *File > Save Data File As...* or *File > Export Image As...* to do so. It is important to note that saving a project/data file or exporting an image can only be done when logging is not active.

## Import register definitions from *mbc* file

*ModbusControl* is an application that can be used to read and write data from Modbus slaves. It is possible to import the register definitions from a *ModbusControl* project file (`.mbc`) into *ModbusScope* by clicking on *Import from .mbc file* in the register dialog or by dragging and dropping the `.mbc` file into the main screen of *ModbusScope*. This makes it easy to add register definitions to *ModbusScope* from a *ModbusControl* project file.

![image](../_static/user_manual/import_from_mbc_dialog.png)
