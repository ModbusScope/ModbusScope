# ModbusScope
[![Build status](https://ci.appveyor.com/api/projects/status/v7ysjn9c2koy1tb8?svg=true)](https://ci.appveyor.com/project/jgeudens/modbusscope)
[![Build Status](https://travis-ci.org/jgeudens/ModbusScope.svg?branch=master)](https://travis-ci.org/jgeudens/ModbusScope)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/19701/badge.svg)](https://scan.coverity.com/projects/jgeudens-modbusscope)

[![Documentation Status](https://readthedocs.org/projects/modbusscope/badge/?version=latest)](https://modbusscope.readthedocs.io/en/latest/?badge=latest)

ModbusScope is a GUI tool to log data using the [Modbus](https://en.wikipedia.org/wiki/Modbus) protocol and put the data into a graph. Data can be examined while the logging continues. It is also possible to export the data to a *.csv* file for analyzing the data afterwards.

![ModbusScope demo](modbusscope_demo.gif)

## Dependencies

ModbusScope depends on the following packages to build.

| Package          | Link                                 |
| :--------------- | :----------------------------------- |
| Qt               | https://www.qt.io/                   |
| QCustomPlot      | https://www.qcustomplot.com/         |
| muParser         | http://muparser.beltoforion.de/      |
| Google Test/Mock | https://github.com/google/googletest |

Qt isn't included in the repository, this should be installed on the build system. Other dependencies are included in the source or added via git submodules.

## Docs

Docs are available at [https://modbusscope.readthedocs.io](https://modbusscope.readthedocs.io).


## Contributing

Please [open a new issue](https://github.com/jgeudens/ModbusScope/issues) to report bugs or [create a pull request](https://github.com/jgeudens/ModbusScope/pulls) to send patches.
