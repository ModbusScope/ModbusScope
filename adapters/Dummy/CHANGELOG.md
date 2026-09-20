# Changelog

## Unreleased

### Added

- Add dummy adapter (`dummyadapter`) for manual ModbusScope testing without real Modbus hardware; identity and capabilities are driven by a hand-edited config file

### Changed

- **Breaking: adapter protocol version 2.** `adapter.readData`'s flat `valid` boolean is replaced by an explicit `state`/`flags` quality model; `adapter.initialize`/`adapter.describe` now negotiate a `protocolVersion`. `ResultState::State`/`isValid()` become `DataQuality::State`/`isUsable()`. See `docs/plan/quality-descriptors-implementation-plan.md` (IEC104Adapter repo).

- Enforce `maxDevices` in `adapter.configure`, rejecting configs whose devices array exceeds it
- `adapter.buildExpression` now returns `${address}` register-reference syntax instead of a bare number
- `shared/` is now owned by this repository instead of being vendored from ModbusAdapter; ModbusAdapter fetches it from here
- Add `licensecore/` (Ed25519 license verification, generic across adapters), owned by this repository the same way `shared/` is; `dummyadapter` itself does not link it or perform a license check
- Release artifacts are now always optimized on both platforms. The Windows CI job never set `BUILD_TYPE`, so `dummyadapter.exe` shipped as an unoptimized Debug build with full symbols while the Linux binary shipped as Release; both now build Release, and optimized builds are linked with `-s`
- Debug-only functionality is gated on a new `ADAPTER_DEBUG_FEATURES` CMake option (default `ON`) rather than on `CMAKE_BUILD_TYPE`, so a small optimized binary can still carry the debug aids. CI turns it off for tag builds only: master and feature branch pre-releases report a git-branch/commit-hash version suffix and keep licensecore's `MODBUSADAPTER_LICENSE_PATH` override, tagged releases report a bare semver and compile the override out
- `adapter.describe` now reports a build-time version (`PROJECT_VERSION`, plus `-<branch>+<hash>` in debug-feature builds) instead of the config file's `version` field, which has been removed from `dummyadapter.json`. `name`, `configVersion` and capabilities stay config-driven
- `DescribeHandler` no longer falls back to a build-time `APP_VERSION` or appends a debug git-branch/hash suffix; callers must always supply a complete version string. `verifyLicense()` now takes the release-train date as a required parameter instead of resolving it from a checked-in `licensecore/traindate.h` (deleted) — both the adapter version and the release-train date are now the consuming protocol repo's responsibility, not this repo's
