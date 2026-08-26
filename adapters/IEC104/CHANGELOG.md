# Changelog

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
