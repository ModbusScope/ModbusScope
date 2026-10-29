# ModbusAdapter License

Copyright (C) 2026 Synvis. All rights reserved.

ModbusAdapter is proprietary software. Use requires a valid license issued by Synvis.

## Relation to ModbusScope

ModbusAdapter is a separate, independent program. It is not part of ModbusScope and is not
covered by ModbusScope's GNU General Public License v3.0.

ModbusAdapter communicates with its host application only through the documented JSON-RPC 2.0
protocol over standard input and output (see `docs/modbus-adapter-spec.md`). It does not link
against, include, or derive from ModbusScope source code. Distributing ModbusAdapter alongside
ModbusScope forms an aggregate as described in section 5 of the GPL-3.0, and does not extend the
GPL-3.0 to ModbusAdapter.

## Qt

ModbusAdapter uses the Qt libraries under the GNU Lesser General Public License v3.0, linked
dynamically. You may replace the Qt libraries it loads with a modified version. Nothing in this
license restricts modification of ModbusAdapter, or reverse engineering for debugging such
modifications, to the extent permitted by section 4 of the LGPL-3.0.

## Third-party components

Notices for other third-party components are listed in `THIRD_PARTY_LICENSES.md`.
