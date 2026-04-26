# Modbus Adapter Implementation Specification

This document defines the Modbus adapter's concrete payloads for the generic methods defined in adapter. Refer to the protocol spec for transport framing, method signatures, the notification and error-code contracts, and the session lifecycle.

Only methods whose request or response shape is Modbus-specific are documented here. Methods not listed (`adapter.initialize`, `adapter.getStatus`, `adapter.readData`, `adapter.shutdown`) use the generic payloads defined in the protocol spec without further specialization.

---

## `adapter.describe`

**Result (Release build):**
```json
{
  "name": "modbusAdapter",
  "version": "0.0.1",
  "configVersion": 1,
  "schema": { ... },
  "defaults": { ... },
  "capabilities": {
    "supportsHotReload": false,
    "requiresRestartOn": ["connections", "devices"]
  }
}
```

**Result (Debug build):** the `version` field appends `-<git-branch>+<commit-hash>`, e.g. `"0.0.1-dev-diagnostics+c471210"`. Branch names with slashes are converted to hyphens (e.g. `dev/diagnostics` → `dev-diagnostics`). Consumers must not treat `version` as a fixed literal; parse or compare it accordingly.

> **Note:** Update this spec whenever the Modbus JSON-RPC implementation changes.

| Field | Description |
| --- | --- |
| `name` | Adapter identifier (`"modbusAdapter"`) |
| `version` | Adapter software version. Release: `"<semver>"`. Debug: `"<semver>-<git-branch-safe>+<commit-hash>"` (slashes in branch name replaced with hyphens). |
| `configVersion` | Current config schema version |
| `schema` | JSON Schema–compatible object describing the `config` object accepted by `adapter.configure` |
| `defaults` | Default config values. The `connections` array contains a single TCP entry that also includes serial-specific fields (`portName`, `baudrate`, `parity`, `databits`, `stopbits`) so callers can see serial defaults without needing a second example. |
| `capabilities` | Feature flags |

The connection schema uses JSON Schema Draft 7 `if`/`then`/`else` to express type-dependent fields. When `type` equals `"tcp"`, the fields in `then.properties` apply (TCP-specific). Otherwise, when `type` is not `"tcp"`, the fields in `else.properties` apply (serial-specific). A UI can use this to enable or disable the relevant fields based on the selected connection type.

---

## `adapter.configure`

Applies Modbus connection and device configuration to the adapter. Must be called before `adapter.start`.

**Params:**
```json
{
  "config": {
    "version": 1,
    "general": {},
    "connections": [
      {
        "id": 0,
        "type": "tcp",
        "ip": "192.168.1.100",
        "port": 502,
        "timeout": 1000,
        "persistent": false
      },
      {
        "id": 1,
        "type": "serial",
        "portName": "/dev/ttyUSB0",
        "baudrate": 115200,
        "parity": "N",
        "databits": 8,
        "stopbits": 1,
        "timeout": 1000,
        "persistent": false
      }
    ],
    "devices": [
      {
        "id": 1,
        "connectionId": 0,
        "slaveId": 1,
        "consecutiveMax": 64,
        "int32LittleEndian": false
      }
    ]
  }
}
```

**General fields:**

Adapter-wide settings. Currently empty; reserved for future use.

| Field | Type | Description |
| --- | --- | --- |
| *(none yet)* | | |

**Connection fields:**

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| `id` | integer | yes | Connection index: `0`, `1`, or `2` |
| `name` | string | no | Human-readable label (default: `"Connection <id>"`) |
| `type` | string | yes | `"tcp"` or `"serial"` |
| `timeout` | integer | no | Timeout in milliseconds (default: `1000`) |
| `persistent` | boolean | no | Keep connection open between reads (default: `true`) |

TCP-specific fields:

| Field | Type | Default | Description |
| --- | --- | --- | --- |
| `ip` | string | `"127.0.0.1"` | IP address |
| `port` | integer | `502` | TCP port |

Serial-specific fields:

| Field | Type | Default | Description |
| --- | --- | --- | --- |
| `portName` | string | | Serial port name (e.g. `/dev/ttyUSB0`, `COM1`) |
| `baudrate` | integer | `9600` | Baud rate |
| `parity` | string | `"N"` | `"N"` (none), `"E"` (even), `"O"` (odd) |
| `databits` | integer | `8` | Data bits: `5`, `6`, `7`, or `8` |
| `stopbits` | integer | `1` | Stop bits: `1` (one), `2` (two), or `3` (one-and-a-half) |

**Device fields:**

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| `id` | integer | yes | Device identifier (unique within the adapter) |
| `connectionId` | integer | yes | Which connection this device is on (`0`–`2`) |
| `slaveId` | integer | yes | Modbus slave ID (`1`–`247`) |
| `consecutiveMax` | integer | no | Max registers per single read request (default: `125`) |
| `int32LittleEndian` | boolean | no | Byte order for 32-bit values (default: `true` = little endian) |

---

## `adapter.start`

Starts Modbus polling with the configuration applied by `adapter.configure`.

**Params:**

```json
{
  "registers": ["${40001: 16b}", "${h0 @ 2: f32b}"]
}
```

Each element is a register subexpression string with the syntax:
`${address [@ deviceId] [: type]}`

**Address format:**

| Prefix | Object type | Example |
| --- | --- | --- |
| `00001`–`09999` or `c` | Coils | `c50`, `00001` |
| `10001`–`19999` or `d` | Discrete inputs | `d100`, `10001` |
| `30001`–`39999` or `i` | Input registers | `i0`, `30001` |
| `40001`–`49999` or `h` | Holding registers | `h0`, `40001` |

**Optional fields:**

| Field | Syntax | Default | Description |
| --- | --- | --- | --- |
| `deviceId` | `@ N` | `1` | Device ID from `adapter.configure` |
| `type` | `: type` | `16b` | Data type |

**Type values:**

| Value | Description |
| --- | --- |
| `"16b"` | Unsigned 16-bit integer (default) |
| `"s16b"` | Signed 16-bit integer |
| `"32b"` | Unsigned 32-bit integer (two consecutive registers) |
| `"s32b"` | Signed 32-bit integer |
| `"f32b"` | 32-bit IEEE 754 float |

---

## `adapter.dataPointSchema`

**Result:**
```json
{
  "addressSchema": {
    "type": "object",
    "properties": {
      "objectType": {
        "type": "string",
        "title": "Object type",
        "enum": ["coil", "discrete input", "input register", "holding register"],
        "x-enumLabels": ["Coil", "Discrete Input", "Input Register", "Holding Register"]
      },
      "address": {
        "type": "integer",
        "title": "Address",
        "minimum": 0,
        "maximum": 65535
      },
      "deviceId": {
        "type": "integer",
        "title": "Device ID",
        "minimum": 1
      },
      "dataType": {
        "type": "string",
        "title": "Data type"
      }
    },
    "required": ["objectType", "address"]
  },
  "dataTypes": [
    { "id": "16b", "label": "unsigned 16-bit" },
    { "id": "s16b", "label": "signed 16-bit" },
    { "id": "32b", "label": "unsigned 32-bit" },
    { "id": "s32b", "label": "signed 32-bit" },
    { "id": "f32b", "label": "32-bit float" }
  ],
  "defaultDataType": "16b"
}
```

The `dataType` property within `addressSchema` has no `enum` constraint; the available values are supplied by the top-level `dataTypes` array, and `defaultDataType` (`"16b"`) indicates which value to pre-select.

---

## `adapter.describeDataPoint`

**Params:**
```json
{
  "expression": "${40001: 16b}"
}
```

**Result (valid):**
```json
{
  "valid": true,
  "fields": {
    "objectType": "holding register",
    "address": 0,
    "deviceId": 1,
    "dataType": "16b"
  },
  "description": "holding register, 0, unsigned 16-bit, device id 1"
}
```

**Result (invalid):**
```json
{
  "valid": false,
  "error": "Unknown type 'xyz'"
}
```

---

## `adapter.validateDataPoint`

**Params:**
```json
{
  "expression": "${40001: 16b}"
}
```

**Result (valid):**
```json
{ "valid": true }
```

**Result (invalid):**
```json
{
  "valid": false,
  "error": "Unknown type 'xyz'"
}
```

---

## `adapter.buildExpression`

**Params:**

```json
{
  "fields": {
    "objectType": "holding register",
    "address": 0
  },
  "dataType": "f32b",
  "deviceId": 2
}
```

**Result:**

```json
{ "expression": "${h0@2:f32b}" }
```

---

## `adapter.expressionHelp`

**Result:**

```json
{ "helpText": "<html>...</html>" }
```

The returned HTML documents the Modbus register expression syntax defined under [`adapter.start`](#adapterstart) (address prefixes, optional `deviceId` and `type`, and the list of type values).

---

## Example

Start a session with a TCP connection on connection 0 and one device, then read a register.

Initialize:

```text
Content-Length: 66\r\n
\r\n
{"jsonrpc":"2.0","id":1,"method":"adapter.initialize","params":{}}
```

Response:

```text
Content-Length: 49\r\n
\r\n
{"id":1,"jsonrpc":"2.0","result":{"status":"ok"}}
```

Describe adapter:

```text
Content-Length: 64\r\n
\r\n
{"jsonrpc":"2.0","id":2,"method":"adapter.describe","params":{}}
```

Response:

```text
Content-Length: <N>\r\n
\r\n
{"id":2,"jsonrpc":"2.0","result":{"name":"modbusAdapter","version":"...","configVersion":1,"schema":{...},"defaults":{...},"capabilities":{...}}}
```

Configure connection and device:

```text
Content-Length: 255\r\n
\r\n
{"jsonrpc":"2.0","id":3,"method":"adapter.configure","params":{"config":{"version":1,"general":{},"connections":[{"id":0,"type":"tcp","ip":"192.168.1.100","port":502,"timeout":1000}],"devices":[{"id":1,"connectionId":0,"slaveId":1,"consecutiveMax":64}]}}}
```

Response:

```text
Content-Length: 49\r\n
\r\n
{"id":3,"jsonrpc":"2.0","result":{"status":"ok"}}
```

Start polling:

```text
Content-Length: 90\r\n
\r\n
{"jsonrpc":"2.0","id":4,"method":"adapter.start","params":{"registers":["${40001: 16b}"]}}
```

Response:

```text
Content-Length: 49\r\n
\r\n
{"id":4,"jsonrpc":"2.0","result":{"status":"ok"}}
```

Read request:

```text
Content-Length: 64\r\n
\r\n
{"jsonrpc":"2.0","id":5,"method":"adapter.readData","params":{}}
```

Response (when Modbus read completes):

```text
Content-Length: 77\r\n
\r\n
{"id":5,"jsonrpc":"2.0","result":{"registers":[{"valid":true,"value":1234}]}}
```
