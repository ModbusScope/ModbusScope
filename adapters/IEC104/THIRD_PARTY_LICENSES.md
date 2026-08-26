# Third-Party Licenses

## lib60870 (v2.4.0)

- Project: lib60870 — IEC 60870-5-101/104 protocol library, MZ Automation GmbH
- Source: <https://github.com/mz-automation/lib60870>, vendored at `libraries/lib60870`
  (managed via `dfetch.yaml`)
- License: **GPL-3.0** — full text at `libraries/lib60870/COPYING`

lib60870 is statically linked into the `iec104adapter` executable (and, through the shared
`AdapterSource` library, into the `dummyiec104adapter` test harness). GPL-3.0 obligations apply
to distribution of these binaries; MZ Automation also offers commercial licensing for
closed-source use.
