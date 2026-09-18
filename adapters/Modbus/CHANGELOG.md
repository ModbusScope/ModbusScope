# Changelog

## Unreleased

### Fixed

- Fix `adapter.readData` staying rejected with "Read already in progress" for up to 5s after `adapter.stop`/`adapter.start` interrupted a Modbus read that was still in flight

### Changed

- Debug-only functionality is gated on a new `ADAPTER_DEBUG_FEATURES` CMake option (default `ON`) rather than on `CMAKE_BUILD_TYPE`. Release artifacts stay fully optimized (and are now linked with `-s`), while CI turns the option off for tag builds only: master and feature branch pre-releases again report the `-<git-branch>+<commit-hash>` version suffix that became unreachable when CI moved to always-Release, and tagged releases report a bare semver
- `MODBUSADAPTER_LICENSE_PATH` is now gated on `DEBUG` instead of `QT_DEBUG`, so it follows `ADAPTER_DEBUG_FEATURES`: present in development builds, compiled out of tagged releases
- Bump the `licensecore` `dfetch.yaml` pin to pick up the `DEBUG` gate change from DummyAdapter (the `shared/` pin is unchanged — those files were not refreshed)
- Remove the stale `shared/licensecore/` tree, a leftover from when `licensecore` lived under `shared/` upstream; nothing referenced it and it held a divergent second copy of `licenseverify.cpp`
- `GIT_BRANCH` now prefers `GITHUB_REF_NAME`; `actions/checkout` leaves a detached HEAD, so CI builds previously reported the literal branch name `HEAD`
- **Breaking: adapter protocol version 2.** `adapter.readData` reports quality via an explicit `state`/`flags` model instead of a flat `valid` boolean; this adapter declares no `capabilities.quality` and never reports a flag, so every point is still simply `good` or `invalid`. `adapter.initialize`/`adapter.describe` negotiate a `protocolVersion`. See `docs/plan/quality-descriptors-implementation-plan.md` (IEC104Adapter repo).

## v0.0.3 - 08/09/2026

### Added

- Add initial implementation of Modbus gateway (disabled for now)
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

### Fixed

- Fixed and updated logging messages
- Fix register list debug log printing a pointer address instead of the register list contents

## v0.0.2 - 07/05/2026

### Added

- Add `adapter.stop` method to stop Modbus polling without terminating the adapter process

### Changed

- Limited number of devices for now
- `adapter.dataPointSchema`:
  - `dataType` in `addressSchema` now includes `enum` and `x-enumLabels`
  - `defaults` now provides the initial `objectType`, `address`, and `dataType` values
  - Available data types are moved inline into the schema property

## v0.0.1 - 25/04/2026

Initial version of the adapter for the Modbus protocol.
