# ModbusScope release checklist

## Release

* Change *DEBUG* to *nDEBUG* (`src/CMakeLists.txt`)
* Check version (`src/CMakeLists.txt`)
* Update version in installer (`installer/modbusscope_installer.iss`)
* Update RELEASE_NOTES.md with changes and release date
* Github
* * Close all implemented issues
  * Close milestone on 

## After release

* Determine next version number
* Update version number of application (`src/CMakeLists.txt`)
* Reset *nDEBUG* to *DEBUG* (`src/CMakeLists.txt`)