# Changelog

## Unreleased

### Added

- Add dummy adapter (`dummyadapter`) for manual ModbusScope testing without real Modbus hardware; identity and capabilities are driven by a hand-edited config file

### Changed

- Enforce `maxDevices` in `adapter.configure`, rejecting configs whose devices array exceeds it
- `adapter.buildExpression` now returns `${address}` register-reference syntax instead of a bare number
