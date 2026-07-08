# Changelog

## Unreleased

### Added

- Add initial implementation of Modbus gateway
- Add license verification
- Add limit of 5 registers in free version
- Add `MODBUSADAPTER_LICENSE_PATH` env var to override the license file path in debug builds

### Changed

- Remove the application name directory from the license file path (now `~/.config/ModbusScope/licenses/<adapter>.lic`)
- License expiry is now checked against the release-train date (the `x.y.0` feature release date) instead of the build date, so security patch releases never invalidate a license that covered the feature release
- Change supported devices to 2 in free version
- Update capabilities
  - Remove unused `supportsHotReload` and `requiresRestartOn`
  - Add `mbcCompatible`

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
