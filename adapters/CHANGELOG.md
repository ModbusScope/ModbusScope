# Changelog

## Unreleased

### Changed

* Update capabilities
  * Remove unused `supportsHotReload`and `requiresRestartOn`
  * Add `mbcCompatible`

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
