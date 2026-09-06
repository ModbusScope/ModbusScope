# Changelog

## Unreleased

### Added

- Add initial implementation of Modbus gateway
- Add license verification
- Add limit of 5 registers in free version
- Add `MODBUSADAPTER_LICENSE_PATH` env var to override the license file path in debug builds
- Update schema to control order of UI elements in the settings dialog

### Changed

- Update the license file path (now `~/.config/ModbusScope/licenses/<adapter>.lic`)
- License expiry is now checked against the release-train date (the `x.y.0` feature release date) instead of the build date, so security patch releases never invalidate a license that covered the feature release
- Change supported devices to 2 in free version
- Update capabilities
  - Remove unused `supportsHotReload` and `requiresRestartOn`
  - Add `mbcCompatible`
- `shared/` is now vendored from [DummyAdapter](https://github.com/ModbusScope/DummyAdapter) via `dfetch` instead of being owned here; generic changes are made there and pulled in by bumping the pin in `dfetch.yaml`
- `licensecore/` is likewise now vendored from DummyAdapter instead of being owned here, including the release-train-date CI guard (`scripts/check_train_date.sh` moved to `licensecore/check_train_date.sh`)
- The adapter version and release-train date are now this repo's own values instead of coming from `shared/`/`licensecore/`: `src/util/version.h.in` computes the reported version, and `src/modbustraindate.h` (`ADAPTER_TRAIN_DATE`) supplies the license expiry reference date. A routine train-date bump no longer requires an upstream DummyAdapter edit

### Fixed

- Fixed and updated logging messages
- Fix register list debug log printing a pointer address instead of the register list contents

## v0.0.2 - (07/05/2026)

### Added

- Add `adapter.stop` method to stop Modbus polling without terminating the adapter process

### Changed

- Limited number of devices for now
- `adapter.dataPointSchema`:
  - `dataType` in `addressSchema` now includes `enum` and `x-enumLabels`
  - `defaults` now provides the initial `objectType`, `address`, and `dataType` values
  - Available data types are moved inline into the schema property

## v0.0.1 - (25/04/2026)

Initial version of the adapter for the Modbus protocol.
