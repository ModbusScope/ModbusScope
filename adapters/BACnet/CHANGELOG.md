
# Changelog

## [unreleased]

Initial version of the adapter for the BACnet protocol.

- Add Ed25519 license verification (`licensecore`). `adapter.initialize` and
  `adapter.describe` now report the license state; no license-dependent limits are
  enforced yet.
- Rename the diagnostic logging categories from `scope.*` to `adapter.*`.
- `shared/` is now vendored from [DummyAdapter](https://github.com/ModbusScope/DummyAdapter) via `dfetch` instead of transitively through ModbusAdapter; generic changes are made there and pulled in by bumping the pin in `dfetch.yaml`
- `licensecore/` is likewise now vendored from DummyAdapter instead of ModbusAdapter, including the release-train-date CI guard (`scripts/check_train_date.sh` moved to `licensecore/check_train_date.sh`)
- The adapter version and release-train date are now this repo's own values instead of coming from `shared/`/`licensecore/`: `src/util/version.h.in` computes the reported version, and `src/bacnettraindate.h` (`ADAPTER_TRAIN_DATE`) supplies the license expiry reference date. A routine train-date bump no longer requires an upstream DummyAdapter edit
