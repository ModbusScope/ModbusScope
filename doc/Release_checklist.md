# ModbusScope release checklist

## General

* Determine version number

## Code

* Update version number of application (`src/CMakeLists.txt`)
* Update version in installer (`installer/modbusscope_installer.iss`)
* Update RELEASE_NOTES.md with changes and release date
* Change *DEBUG* to *NDEBUG* (`src/CMakeLists.txt`)

## Github

* Close all implemented issues
* Close milestone