# ModbusScope release checklist

## Release

* Check version (`src/CMakeLists.txt`)
* Update version in installer (`installer/modbusscope_installer.iss`)
* Update RELEASE_NOTES.md with changes and release date
* Github

* * Close all implemented issues
  * Close milestone on 

## After release

* Determine next version number
* Update version number of application (`src/CMakeLists.txt`)
* Reset *NDEBUG* to *DEBUG* (`src/CMakeLists.txt`)