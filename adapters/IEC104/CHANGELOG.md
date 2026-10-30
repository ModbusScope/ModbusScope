# Changelog

## Unreleased

### Added

- `adapter.describe` reports a user-facing `protocolName` (`"IEC104"`) alongside the stable `name` identifier
- `adapter.describe` config schema marks `devices[].connectionId` with `x-ref` (`collection`, `value`, `label`) so hosts can render a connection picker; update vendored `shared/` to the latest DummyAdapter

### Changed

- `adapter.inspectLicense` is answered with JSON-RPC error `-32601` Method not found (no license verification in this adapter)

## v0.0.3 - 01/10/2026

### Changed

- **Breaking: adapter protocol version 2.** `adapter.readData` reports QDS quality via an explicit `state`/`flags` model instead of a flat `valid` boolean; `OV`/`SB`/`BL` now arrive usable and labelled instead of being discarded. `adapter.initialize`/`adapter.describe` negotiate a `protocolVersion`.
- Always build Release in CI; update vendored `shared/` to the latest DummyAdapter

### Fixed

- Stale quality flags surviving a clean update; `addFlags()` wrongly demoting Good to Degraded (via `shared/` bump)

## v0.0.2 - 15/09/2026

### Changed

- Disable verbose communication logging

## v0.0.1 - 08/09/2026

### Added

- Initial IEC 60870-5-104 adapter: JSON-RPC 2.0 stdio interface (vendored `shared/` tree),
  data-mirror communication architecture, `${ioa[@deviceId]}` datapoint expressions,
  connection/device configuration schema
- Deterministic simulated IEC 104 client (`SimulatedIec104Client`) behind the `IIec104Client`
  interface
- Verbose logging of the IEC 104 messages crossing the client boundary (transmitted connect/close,
  general interrogations; received points with type, addresses, cause of transmission, value and
  quality, incl. points without a matching datapoint), behind the `ADAPTER_VERBOSE_COMM_LOGGING`
  build option (~~off by default~~)
- Real IEC 104 client (`Lib608s70Iec104Client`) backed by the vendored lib60870 v2.4.1 stack
  (GPL-3.0, threadless CS104 API driven on a dedicated worker thread) as the production default,
  plus an in-process CS104 test slave and client↔slave integration tests
