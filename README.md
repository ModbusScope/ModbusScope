# ModbusScope
ModbusScope is an QT application that can be used to log data using the modbus protocol and put the data into a nice graph. It is also possible to export the data or even an image of the graph.

## Dependencies
ModbusScope depends on two libraries:
* [libmodbus](http://libmodbus.org/)
* [QCustomPlot](http://www.qcustomplot.com/)


## Cross-platform
ModbusScope aims to be cross platform. Main development is currently done in linux. But the release are done for Windows. Releases are statically linked with the QT library. This ensures that the ModbusScope runs smoothly on windows without installing any libraries.

## Protocols
Currently only modbus TCP is supported. Eventually modus RTU will also be supported.


