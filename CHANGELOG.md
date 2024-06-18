The latest *ModbusScope* installer or standalone version can always be downloaded from the [release page](https://github.com/jgeudens/ModbusScope/releases).

## [v4.0.0](https://github.com/jgeudens/ModbusScope/releases/tag/4.0.0) (18/06/2024)

### Added

* Add extended register syntax to handle full address range (e.g `${h65535}`)
* Highlight expression syntax error in register dialog
* Implement easier editing of expression in register dialog

### Fixed

* Fixed warning when loading mbs file with unsupported datalevel
* Don't deselect axis when Ctrl key is pressed ([Github #304](https://github.com/jgeudens/ModbusScope/issues/304))

### Changed

* Improve formatting of large and small values ([Github #287](https://github.com/jgeudens/ModbusScope/issues/287))

### Removed

* Removed correction of corrupt values (STMStudio)
* Removed support for opening very old mbs files (ModbusScope v2.x.x or earlier)
  * Old mbs files can be updated by opening and saving using ModbusScope v3.x.x.

## [v3.9.0](https://github.com/jgeudens/ModbusScope/releases/tag/3.9.0) (04/12/2023)

### Added

* Add support for other object types (discrete output coils, discrete input contacts, input registers, holding registers)

## [v3.8.1](https://github.com/jgeudens/ModbusScope/releases/tag/3.8.1) (12/08/2023)

### Added

* Allow in place editing of expressions

### Changed

* Allow screenshot during logging ([Github #280](https://github.com/jgeudens/ModbusScope/issues/280))

## [v3.8.0](https://github.com/jgeudens/ModbusScope/releases/tag/3.8.0) (02/06/2023)

### Added

* Improve visibility of errors (highlight curve in legend)

### Fixed

* Fixed diagnostic log cleared when "extensive logging" is disabled ([Github #265](https://github.com/jgeudens/ModbusScope/issues/265))

### Changed

* Change add register dialog (wizard) to drop-down frame
* Keep focus of selected register when filtering during mbc import ([Github #169](https://github.com/jgeudens/ModbusScope/issues/169))

## [v3.7.0](https://github.com/jgeudens/ModbusScope/releases/tag/3.7.0) (03/02/2023)

### Changed

* Rework toolbar (new icons and remove some actions)
* Update notification is now only visible after 14 days since release
* Improve marker indicators (Z-order)
* Select value axis scale options tab on axis selection ([Github #253](https://github.com/jgeudens/ModbusScope/issues/253))
* Update dependencies

### Added

* Add support for 32-bit floating point type ([Github #250](https://github.com/jgeudens/ModbusScope/issues/250))
* Add indicator on axis to indicate value axis configuration of curve

## [v3.6.3](https://github.com/jgeudens/ModbusScope/releases/tag/3.6.3) (21/11/2022)

### Fixed

* Fix menu on incorrect screen ([Github #248](https://github.com/jgeudens/ModbusScope/issues/248))

## [v3.6.2](https://github.com/jgeudens/ModbusScope/releases/tag/3.6.2) (06/11/2022)

### Fixed

* Fix marker data calculations ([Github #240](https://github.com/jgeudens/ModbusScope/issues/240))
* Fix markers in combination with value axis ([Github #240](https://github.com/jgeudens/ModbusScope/issues/240))

## [v3.6.1](https://github.com/jgeudens/ModbusScope/releases/tag/3.6.1) (04/10/2022)

### Fixed

* Fix zooming of axis with mouse wheel ([Github #234](https://github.com/jgeudens/ModbusScope/issues/234))

## [v3.6.0](https://github.com/jgeudens/ModbusScope/releases/tag/3.6.0) (02/10/2022)

### Changed

* When starting only set scaling to auto when it is set to manual ([Github #210](https://github.com/jgeudens/ModbusScope/issues/210))
* Improve import/export of csv data (especially when modifying file in Excel) ([Github #220](https://github.com/jgeudens/ModbusScope/issues/220))
* Remove unused space in scale dock

### Added

* Second Y-axis on right-side ([Github #188](https://github.com/jgeudens/ModbusScope/issues/188))
* Add option to quickly add a register

### Fixed

* Fix blurry tick labels in Qt6 build
* Fix crash when there is no data and marker is active ([Github #223](https://github.com/jgeudens/ModbusScope/issues/223))
* Fix crash when a register is added and data is present ([Github #229](https://github.com/jgeudens/ModbusScope/issues/229))

## [v3.5.1](https://github.com/jgeudens/ModbusScope/releases/tag/3.5.1) (11/05/2022)

* Crash on showing tooltip with empty graphs ([Github #208](https://github.com/jgeudens/ModbusScope/issues/208))
* Incorrect tab stop order when adding new registers ([Github #209](https://github.com/jgeudens/ModbusScope/issues/209))
* Revert Windows build back to Qt5.15.2 to fix blurry tick labels

## [v3.5.0](https://github.com/jgeudens/ModbusScope/releases/tag/3.5.0) (04/05/2022)

* Convert min/max settings of Y axis to floating point ([Github #183](https://github.com/jgeudens/ModbusScope/issues/183))
* Add *Save Project* menu item
* Implement changing order of registers by drag and drop ([Github #78](https://github.com/jgeudens/ModbusScope/issues/78))
* Allow scrolling to negative times when there is negative time data ([Github #198](https://github.com/jgeudens/ModbusScope/issues/198))
* Update to Qt6

## [v3.4.0](https://github.com/jgeudens/ModbusScope/releases/tag/3.4.0) (02/03/2022)

* Allow full 16-bit addressing of holding registers ([Github #181](https://github.com/jgeudens/ModbusScope/issues/181))
* Expand expressions with register definition to provide more flexibility for user ([Github project #6](https://github.com/jgeudens/ModbusScope/projects/6))
  * This changes adds the ability to combine multiple registers from potential multiple connections in one curve.
* Add syntax highlighting to expressions, including invalid token ([Github #142](https://github.com/jgeudens/ModbusScope/issues/142))

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
When loading old ModbusScope files (pre v3.x.x), the existing operations (multiply, divide, shift, bitmask, ...) will be converted to a single expression that is used in the new ModbusScope. This conversion makes sure that users won't notice any differences in functionality.

When exporting the settings (project file), the new expression will be saved. Older ModbusScope versions won't be able to parse this expressions. The operations will be reset to the defaults, but other register info will be loaded correctly.

## [v2.1.1](https://github.com/jgeudens/ModbusScope/releases/tag/2.1.1) (03/07/2020)

* Fix update check (add OpenSSL dll to install) ([Github #136](https://github.com/jgeudens/ModbusScope/issues/136))

## [v2.1.0](https://github.com/jgeudens/ModbusScope/releases/tag/2.1.0) (15/06/2020)

**Defects**

* Fix tooltip

**Improvements**

* Add support for 32 bit registers ([Github #129](https://github.com/jgeudens/ModbusScope/issues/129))
* Add support for persistent connection (default on) ([Github #18](https://github.com/jgeudens/ModbusScope/issues/18))
* Minimize scale dock
* Disable bit mask for signed numbers
* Rework logging to be able improve logging in the future

## [v2.0.0](https://github.com/jgeudens/ModbusScope/releases/tag/2.0.0) (03/03/2020)

**Internal (code changes)**

* Add more tests
* Fix most issues reported by static code analysis (Coverity)
* Change linking from static to dynamic
  * Application changes from one large executable to a smaller executable with extra dll's

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
  * Improve file loading windows (keep showing raw data on invalid parse settings) ([Github #120](https://github.com/jgeudens/ModbusScope/issues/120))
  * Speed up loading large data file ([Github #121](https://github.com/jgeudens/ModbusScope/issues/121))
  * Add progress bar on file load ([Github #122](https://github.com/jgeudens/ModbusScope/issues/122))

**Bugs**

* Small bug fixes ([Github #111](https://github.com/jgeudens/ModbusScope/issues/111))

## v1.6.1 (06/04/2019)

**Bugfixes**

* Fix error when writing notes to imported data file ([Github #109](https://github.com/jgeudens/ModbusScope/issues/109))
* Automatically remove field separator from register names ([Github #106](https://github.com/jgeudens/ModbusScope/issues/106))

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
* Add tab filter when importing mbc files (implements [Github #96](https://github.com/jgeudens/ModbusScope/issues/96))

**Bug fixes**

* Fix slow drag issue ([Github #104](https://github.com/jgeudens/ModbusScope/issues/104))
* Keep visibility state when adding/removing graphs ([Github #102](https://github.com/jgeudens/ModbusScope/issues/102))
* Absolute time is off by 2 hours ([Github #103](https://github.com/jgeudens/ModbusScope/issues/103))

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
* Add option to show/hide all graphs ([Github #99](https://github.com/jgeudens/ModbusScope/issues/99))
* Add logging ([Github #71](https://github.com/jgeudens/ModbusScope/issues/71))

**Bugfixes**

* Fix some minor bugs ([Github #95](https://github.com/jgeudens/ModbusScope/issues/95)) (thanks to @pluyckx), ([Github #89](https://github.com/jgeudens/ModbusScope/issues/89))

## v1.3.0 (01/04/2017)

**Features**

* Rework tooltip (show value under cursor in legend when control key is pressed) ([Github #90](https://github.com/jgeudens/ModbusScope/issues/90))
* Import registers from mbc file (drag and drop or button in register dialog) ([Github #91](https://github.com/jgeudens/ModbusScope/issues/91))
* Add window auto scale on y-axis ([Github #36](https://github.com/jgeudens/ModbusScope/issues/36))
* Add meta data when exporting data (keep color when importing) ([Github #63](https://github.com/jgeudens/ModbusScope/issues/63))
* Added extra marker calculations (minimum, maximum, average, median, slope, ...) ([Github #79](https://github.com/jgeudens/ModbusScope/issues/79))
* Use delete button to remove registers ([Github #34](https://github.com/jgeudens/ModbusScope/issues/34))
* Improve communication (only split Modbus read on specific Modbus exception)
* Update QModbusPlot to v2.0.0 (beta)
* Add command line argument to enable OpenGL (--opengl)

**Bugs**

* Make sure legend window (when docked) is present on screenshot ([Github #80](https://github.com/jgeudens/ModbusScope/issues/80))
* Small fixes ([Github #82](https://github.com/jgeudens/ModbusScope/issues/82), [Github #83](https://github.com/jgeudens/ModbusScope/issues/83), [Github #85](https://github.com/jgeudens/ModbusScope/issues/85), [Github #88](https://github.com/jgeudens/ModbusScope/issues/88))

## Older releases

Older releases can be found on [Github](https://github.com/jgeudens/ModbusScope/releases)
