# ModbusScope release checklist

## Release

* Change *DEBUG* to *nDEBUG* (`src/CMakeLists.txt`)
* Check version (`src/CMakeLists.txt`)
* Update version in installer (`installer/modbusscope_installer.iss`)
  * Remove Beta suffix
* Update RELEASE_NOTES.md with changes and release date
* Github
  * Close all implemented issues
  * Close milestone on 

## After release

* Change *nDEBUG* to *DEBUG* (`src/CMakeLists.txt`)
* Determine next version number
* Update version number of application (`src/CMakeLists.txt`)
* Set version in installer (`installer/modbusscope_installer.iss`) to `[new version] Beta`
* Prepare release notes for new dev cycle

### Release notes

```
## [Unreleased]

### Added 

- xx

### Fixed

- xx

### Changed

- xx

### Removed

- xx
```

