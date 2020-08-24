# Build status
[![Build status](https://ci.appveyor.com/api/projects/status/v7ysjn9c2koy1tb8?svg=true)](https://ci.appveyor.com/project/jgeudens/modbusscope)
[![Build Status](https://travis-ci.org/jgeudens/ModbusScope.svg?branch=master)](https://travis-ci.org/jgeudens/ModbusScope)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/19701/badge.svg)](https://scan.coverity.com/projects/jgeudens-modbusscope)

# What is ModbusScope
ModbusScope is an application that can be used to log data using the [Modbus](https://en.wikipedia.org/wiki/Modbus) protocol and put the data into a nice graph. It is also possible to export the data or an image of the graph.

![ModbusScope demo](docs/_static/modbusscope_demo.gif)

# Project 

## Libraries
ModbusScope is written using Qt. The Qt implementation of Modbus is used to communicate with a slave. The other main library that is used is [QCustomPlot](http://www.qcustomplot.com/). This widget provides the possibility to display the data in a new way. 

## Testing
Automated testing is added to the project recently. Both unit testing and integration are being added.

## Cross-platform
ModbusScope aims to be cross platform. Main development is currently done in linux. But the release are done for Windows. Releases are statically linked with the Qt library. This ensures that the ModbusScope runs smoothly on windows without installing any libraries.
