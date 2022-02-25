# Release notes

The latest *ModbusScope* installer or standalone version can always be downloaded from the [release page](https://github.com/jgeudens/ModbusScope/releases).

## v3.4.0 (unreleased)

* Allow full 16-bit addressing of holding registers ([Github #181](https://github.com/jgeudens/ModbusScope/issues/181))

## [v3.3.1](https://github.com/jgeudens/ModbusScope/releases/tag/3.3.1) (23/12/2021)

* Select next register row after deletion in register dialog
* Improve note positioning when sliding window ([Github #168](https://github.com/jgeudens/ModbusScope/issues/168))
* Hide markers on data load ([Github #171](https://github.com/jgeudens/ModbusScope/issues/171))
* Fix opening project file via menu ([Github #173](https://github.com/jgeudens/ModbusScope/issues/173))
* Let text filter also work on register number during mbc import ([Github #174](https://github.com/jgeudens/ModbusScope/issues/174))

## [v3.3.0](https://github.com/jgeudens/ModbusScope/releases/tag/3.3.0) (08/10/2021)

* Various user experience improvements
  * Replace check boxes with radio buttons
  * Rename menu items
  * Add extra menu items to tool bar
* Reload previous mbc file when import mbc dialog is opened ([Github #156](https://github.com/jgeudens/ModbusScope/issues/156))
* Improve documentation (fixed [Github #161](https://github.com/jgeudens/ModbusScope/issues/161) and open data file chapter)
* Fix that marker points are visible when graph isn't ([Github #157](https://github.com/jgeudens/ModbusScope/issues/157))
* Include connection id in duplicate check when opening mbs file ([Github #164](https://github.com/jgeudens/ModbusScope/issues/164))
* Updated used libraries and internal improvements

## [v3.2.1](https://github.com/jgeudens/ModbusScope/releases/tag/3.2.1) (15/04/2021)

* Fix crash when starting ModbusScope with project file ([Github #155](https://github.com/jgeudens/ModbusScope/issues/155))

## [v3.2.0](https://github.com/jgeudens/ModbusScope/releases/tag/3.2.0) (03/04/2021)

**Improvements**
* Add more visible update notification
* Make documentation and project page (with issue) more visible ([Github #152](https://github.com/jgeudens/ModbusScope/issues/152))

**Fixes**
* When data file is loaded, reset value in legend after inspection with `Control` key
* Reset scaling to auto when clearing data

## [v3.1.0](https://github.com/jgeudens/ModbusScope/releases/tag/3.1.0) (23/02/2021)

**Improvements**
* Add support for modbus RTU (serial port)
* Add options to export diagnostic log
  * Copy specific logs to clipboard
  * Export complete log to file ([Github #135](https://github.com/jgeudens/ModbusScope/issues/135))
* Minor graphical tweaks to markers

## [v3.0.0](https://github.com/jgeudens/ModbusScope/releases/tag/3.0.0) (14/11/2020)

**Improvements**
* Replace fixed operations (multiply, divide, shift, ...) with custom user-defined expression
  * [Link to doc](https://modbusscope.readthedocs.io/en/latest/pages/configuration.html#expressions)
  * Fixed operations will be automatically converted to custom expression on project load
* Rework and move the user manual to [ReadTheDocs](https://modbusscope.readthedocs.io/en/stable/)
* Expand information in logs

* Improve handling of large time periods ([Github #139](https://github.com/jgeudens/ModbusScope/issues/139))
  * Don't wrap around when period is larger than one day

**Backward compatibility**
When loading old ModbusScope files  (pre v3.x.x), the existing operations (multiply, divide, shift, bitmask, ...) will be converted to a single expression that is used in the new ModbusScope. This conversion makes sure that users won't notice any differences in functionality.

When exporting the settings (project file), the new expression will be saved. Older ModbusScope versions won't be able to parse this expressions. The operations will be reset to the defaults, but other register info will be loaded correctly.

## [v2.1.1](https://github.com/jgeudens/ModbusScope/releases/tag/2.1.1) (03/07/2020)

* Fix update check (add OpenSSL dll to install) (##136)

## [v2.1.0](https://github.com/jgeudens/ModbusScope/releases/tag/2.1.0) (15/06/2020)

**Defects**

* Fix tooltip

**Improvements**
* Add support for 32 bit registers (##129)
* Add support for persistent connection (default on) (##18)
* Minimize scale dock
* Disable bit mask for signed numbers
* Rework logging to be able improve logging in the future

## [v2.0.0](https://github.com/jgeudens/ModbusScope/releases/tag/2.0.0) (03/03/2020)

**Internal (code changes)**
- Add more tests
- Fix most issues reported by static code analysis (Coverity)
- Change linking from static to dynamic
  - Application changes from one large executable to a smaller executable with extra dll's

**Features**
* Added possibility to poll 2 different slaves in the same log
* Added possibility to change graph color from legend
* Added filter (error/info) in diagnostics window
* Added mbc filter based on description / register number
* Toggle markers option
* Rectangle zoom
* Integrated most used features from GraphViewer
  * More flexible configuration of parsing settings
  * Advanced auto detection of parsed settings
  * Presets of parse settings
* Improved file loading/parsing
  * Improve file loading windows (keep showing raw data on invalid parse settings) (##120)
  * Speed up loading large data file (##121)
  * Add progress bar on file load (##122)

**Bugs**
* Small bug fixes (##111)

## v1.6.1 (06/04/2019)

**Bugfixes**
* Fix error when writing notes to imported data file (##109)
* Automatically remove field separator from register names (##106)

**Improvements**
* When importing a mbc file already selected registers are now disabled dynamically.

## v1.6.0 (25/01/2019)
Most of the work in this release isn't visible for the user. A complete rework of the communication module has been done. libmodbus was dropped in favor of Qt Modbus. The communication module has also been completely reworked to be able to implement integration and unit testing.

## v1.5.0 (02/10/2018)
**Features**
* Add support for notes (small texts in graph)
  * Editable (even after data load)
  * Saved with data in csv
* Update legend component (thanks to @Fornax)
* Add tab filter when importing mbc files (implements ##96)

**Bug fixes**
* Fix slow drag issue (##104)
* Keep visibility state when adding/removing graphs (##102)
* Absolute time is off by 2 hours (##103)

**Under the hood**
* Add initial unit tests for some modules
* Add Travis build

## v1.4.0 (17/02/2018)
**Features**
* Rework Modbus communication code
* Update libraries
  * QCustomPlot v2.0.0 (final)
  * libmodbus to v3.14
* Improve support for absolute timestamp
* Add option to show/hide all graphs (##99)
* Add logging (##71)

**Bugfixes**
* Fix some minor bugs ##95 (thanks to @pluyckx), ##89

## v1.3.0 (01/04/2017)
**Features**
- Rework tooltip (show value under cursor in legend when control key is pressed) (##90)
- Import registers from mbc file (drag and drop or button in register dialog) (##91)
- Add window auto scale on y-axis (##36)
- Add meta data when exporting data (keep color when importing) (##63)
- Added extra marker calculations (minimum, maximum, average, median, slope, ...) (##79)
- Use delete button to remove registers (##34)
- Improve communication (only split Modbus read on specific Modbus exception)
- Update QModbusPlot to v2.0.0 (beta)
- Add command line argument to enable OpenGL (--opengl)

**Bugs**
- Make sure legend window (when docked) is present on screenshot (##80)
- Small fixes (##82, ##83, ##85, ##88)

## Older releases

Older releases can be found on

[Github]: https://github.com/jgeudens/ModbusScope/releases

