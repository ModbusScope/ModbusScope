# Configuration

## Configure register settings

When opening *ModbusScope*, no Modbus registers are added . First click on *Register Settings* in the toolbar of the interface. This will open a window where registers can be added and edited.

### Add Modbus registers

In the *Register settings* dialog, Modbus registers can be added either manually or by importing from a *.mbc* file. When a *.mbs* file is opened, some registers will already be present in the dialog.

When the registers are added, they can be adjusted afterwards. Such as updating the name and changing to a specific color. An expression is used to define which data is added to the graph (and log). These expressions consist of a calculation which can contains several Modbus register for multiple connections. The user is free to combine calculations and registers in one valid expression. This allows the user to log the data in a format that is clear for the user.

The Modbus standard defines the range for the holding register as 40001 to 49999. But the offset 40001 is removed in the data that will be send in the packet. Since the field for the address is 16-bit, the actual possible range is 40001 to 105536. ModbusScope supports the full range to support as many devices as possible.

> **NOTE**: The number of registers that are polled significantly impacts the sample rate. For higher resolution in time, reduce the number of registers that are actively polled or make sure the register are in subsequent addresses so they can be polled in one packet.

![image](../_static/user_manual/register_settings_dialog_with_registers.png)

![image](../_static/user_manual/add_register_dialog.png)

### Compose expression window

As mentioned before, an expression can be used to transform the raw value from the device to something more understandable to the user. This expression can be tested in the *compose expression* window. This windows can be opened by double clicking the expression cell.

The expression can be freely updated. The register definition will be validated while entering. When the register definition is green, the register definition is valid. The example input table can be used to enter values to test and verify the expression with actual values.

![image](../_static/user_manual/expression_dialog.png)

#### Expressions

A Modbus register read is represented as `${REG\[@CONN]\[:TYPE]}`. Multiple registers read can be combined in one expression.

* `${45332}` => 16-bit unsigned using connection 1
* `${45332: s16b}` => 16-bit signed using connection 1
* `${45332@2: 32b}` => 32-bit unsigned using connection 2
* `${45332@2}` => 16-bit unsigned using connection 2

The most common binary operators are supported (`!`, `|`, `&`, `<<`, `>>`). The basic arithmetic operators are also supported (`+`, `-`, `*`, `/`, `%`, `^`). Hexadecimal numbers can be represented with the `0x`. Binary are represented with `0b` prefix. Floating point number are also supported. Both a decimal point as comma can be used. The first encountered character is used as floating point separator.

Some examples:

* `${40001: s16b} + ${40002@2} * 2`
* `${40001: s32b} * 1.1`
* `${40001} + 0x1000`
* `${40001} & 0b11111000`
* `()${40001} >> 8) & 0xFF`

#### Expression error

When the expression contains an error or when the combination of the expression with a specific input value generates an error, no output value is shown. A specific error message will be shown and the register definition will be indicated in red.

![image](../_static/user_manual/expression_dialog_error.png)

## Configure connection settings

A Modbus connection needs to configured correctly before any data can be read. In the *connection setting* window, up to 3 connections can be configured. Multiple connections means that several Modbus slaves can be polled in a single log session. A register can be coupled to a specific connection in the *register settings* window. ModbusScope supports 2 types of Modbus communication: TCP and RTU. Modbus ASCII isn't supported. A connection type can be selected per connection.

![image](../_static/user_manual/connection_settings.png)

There are two types of connection settings. Some connection settings (*IP*, *port*, *port name*, *baud rate*, *parity*, ... ) are specific to the type of connection. The other settings (*slave ID*, *timeout*, *max consecutive register* and *32-bit little endian*) are specific to the Modbus protocol implementation in the device. The *persistent connection* option is specific to *ModbusScope*. When enabled, *ModbusScope* will keep the connection open between polling data points. The connection will only be reinitialized when a connection error occurs.

For the TCP connection, you need to specify the correct IP address and port of the connection to the slave. For the serial connection, you will need to select the name of the serial port in the drop down list. Next you will need to specify some more settings like baud rate, parity, number of stop bits and number of data bits.

## Configure log settings

*ModbusScope* will create a data file in the OS temporary folder when a logging session is started by default. *ModbusScope* will append data points during the logging to this file. When the user forgets to save the data before quitting or when an unforeseen crash occurs, the data can be recovered. The temporary file is cleared every time a polling session is started. In the *log settings* window, this behavior can be disabled or the temporary data file can be changed.

![image](../_static/user_manual/log_settings.png)

By default, *ModbusScope* will log data points every 250 milliseconds. This can be adjusted in this *log settings* windows. *ModbusScope* will log timestamps relative to the start of the log session by default. By enabling the *use absolute times* option, absolute timestamps are logged instead.

### Optimize logging interval

The minimum logging interval is determined by several factors. The Modbus protocol has an inherent slowdown when register addresses aren't in successive order. When the register addresses aren't successive, *ModbusScope* will split the read request in several packets. But this will negatively impact the minimum logging interval because of the Modbus end of frame timeout. When a fast logging interval is required then limit the number of registers and make sure all register are in order.
