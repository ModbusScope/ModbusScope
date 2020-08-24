# ModbusScope - User Manual

This user manual describes how to get started with *ModbusScope*.

## Installing

*ModbusScope* can be installed with the provided *.msi* installer. Double-click it and follow the instructions. 

## Using

*ModbusScope* is a program which can sample Modbus registers and plot the value in a graph to visualize and investigate the dynamic behavior of a system.

## Adding Modbus registers

When opening *ModbusScope*, no Modbus registers are added. First click on *Register Settings* in the toolbar of interface. This will open a menu where registers can be added.

<img src="../_static/user_manual/register_settings.png" style="zoom: 67%;" />

In the below dialog Modbus registers can be added either manually or imported from *.MBC* file.

### Import mbc file

The *.MBC* file is provided and used for the registers in *ModbusControl*.  Since this is the easiest way, click on *Import from MBC file*. You can also open this window by dragging and dropping the MBC file in the main screen of *ModbusScope*.

<img src="../_static/user_manual/register_settings_dialog.png" style="zoom:67%;" />

By pressing the *...* button in the top right corner the path to an *.mbc* can be given. By using either the *Text filter* or scrolling through the list, find and mark each register for which the value must be added to the graph. If all registers that you want to inspect are selected, press *OK*.

> **NOTE**: The number of registers that are polled significantly impacts the refresh rate. For higher resolution in time, reduce the number of registers added.

<img src="../_static/user_manual/import_from_mbc_dialog.png" style="zoom:67%;" />

Now the registers are added they can be adjusted. Such as updating the name, adding a specific color or adjusting for multiplication factors etc. If all is set press *OK*. 

<img src="../_static/user_manual/register_settings_dialog_with_registers.png" style="zoom:67%;" />

The added registers are now shown in the *Legend* (right hand side of screen). 

### Register calculations

Some calculations can be performed before the data is added to the graph (and log). This calculations can be used to transform the values from a device to a format that is more clear for the user.

##### Order

The calculations are done in a specific order.

* Combine 2 registers to one (32 bit registers)
* Signed/unsigned
* Bitmask (is ignored for signed values)
* Shift
* Multiplication
* Division

## Storing and reusing configuration

Typically the same set of registers are needed. The configuration of registers as described in the previous section can be stored and reloaded through an *.mbs* file. Storing the settings as *.mbs* file can be done through the *File > Export Settings...* and reloading them from the *.mbs* file can be done through *File > Load Project File...* 

## Logging values

Once some registers are added, the actual logging can be done. *ModbusScope* tries to communicate through the active *ModbusControl* instance. Make sure *ModbusControl* is set-up and working correctly.

Then press the  *Start Logging* button (▶-icon) . *ModbusScope* will start logging and  automatically add the values to the graph.

> **NOTE**: When restarting the logging using the *Start Logging* button any data already present in the graph will be cleared.

<img src="../_static/user_manual/start_logging.png" style="zoom:67%;" />

Once your test is finished the logging can be stopped by pressing the *Stop Logging* button (■-icon). The logging will stop and the values can be inspected.

<img src="../_static/user_manual/stop_logging.png" style="zoom:67%;" />

## Exporting data/image

To store any results these can be exported either as image or as data (*.csv* ) file. This can be selected with either *File > Export Data...* or  *File > Export Image...* .

<img src="../_static/user_manual/export_data.png" style="zoom:67%;" />

## Using markers

*ModbusScope* can help in investigating dynamic behavior of a system by measuring times and value differences between two points in the graph. First two vertical markers have to be added.

Add the left marker (green vertical line) by pressing *<Ctrl>* and *<Left mouse click>* on the place of interest. The position must coincide with a sample in the graph. Add the right marker (red vertical line) by pressing *<Ctrl>* and *<Right mouse click>* on the place of interest. 

On the right hand side the *Markers* panel will show information about the markers. Such as the value of the registers at *Time 1* (left hand marker) and *Time 2* (right hand marker). Also the time difference between the two markers or value difference between the two marked times.

<img src="../_static/user_manual/markers.png" style="zoom:67%;" />