# ModbusScope release checklist

## Release

* Check the version number (`CMakeLists.txt`)
  * The version is the single source of truth and is propagated automatically to
    `src/util/version.h.in` and `resources/modbusscope.rc.in`, and is read from the
    built binary by the Windows installer.
* Finalize `CHANGELOG.md`
  * Make sure the top entry uses the version being released and set the release date
* Github
  * Close all implemented issues
  * Close the milestone
* Tag the release (e.g. `5.0.0`)
  * CI builds the artifacts in *Release* mode automatically for tags
    (see `.github/workflows/ccpp.yml`); no manual build-type change is needed.
* Create the Github release and attach the build artifacts

## After release

* Determine the next version number
* Update the version number of the application (`CMakeLists.txt`)
* Prepare release notes for the new dev cycle by adding an `## [Unreleased]` section to
  the top of `CHANGELOG.md`

### Release notes template

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
