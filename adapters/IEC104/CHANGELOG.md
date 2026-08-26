# Changelog

## Unreleased

### Added

- Initial IEC 60870-5-104 adapter: JSON-RPC 2.0 stdio interface (vendored `shared/` tree from
  ModbusAdapter), data-mirror communication architecture, `${ioa[@deviceId]}` datapoint
  expressions, connection/device configuration schema
- Deterministic simulated IEC 104 client (`SimulatedIec104Client`) behind the `IIec104Client`
  interface
- `dummyiec104adapter` test harness and `schemadump` tool
- Verbose logging of the IEC 104 messages crossing the client boundary (transmitted connect/close,
  general interrogations; received points with type, addresses, cause of transmission, value and
  quality, incl. points without a matching datapoint), behind the `ADAPTER_VERBOSE_COMM_LOGGING`
  build option (off by default)
- Real IEC 104 client (`Lib60870Iec104Client`) backed by the vendored lib60870 v2.4.0 stack
  (GPL-3.0, threadless CS104 API driven on a dedicated worker thread) as the production default,
  plus an in-process CS104 test slave and client↔slave integration tests

### Changed

- `SimulatedIec104Client` is now used only by unit tests and `dummyiec104adapter`; the
  `iec104adapter` binary talks to real stations via lib60870
- Update the vendored `shared/` tree: the logging singleton moved from `ScopeLogging` to
  `AdapterLogging` and its `scope.*` logging categories are now `adapter.*`; `DescribeHandler`
  takes the capabilities as a callable, so they are built on every `adapter.describe` call
- Log adapter startup and shutdown, and a summary of every applied configuration
