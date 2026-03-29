# ModbusAdapter JSON-RPC 2.0 Specification

## Overview

The ModbusAdapter process communicates with a host application (client) via JSON-RPC 2.0 over its standard input and output streams.

The adapter is launched as a child process by the client. The client writes JSON-RPC requests to the adapter's **stdin** and reads responses and notifications from its **stdout**.

---

## Transport

### Message Framing

All messages are framed using a Content-Length header:

```text
Content-Length: <N>\r\n
\r\n
<N bytes of UTF-8 JSON>
```

- The header and body are separated by `\r\n\r\n`.
- `<N>` is the exact byte length of the JSON body.
- No other headers are defined.

### Message Types

**Request** (client → adapter):
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "adapter.initialize",
  "params": {}
}
```

**Response** (adapter → client):
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {}
}
```

**Error response** (adapter → client):
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "error": {
    "code": -32602,
    "message": "Invalid params: ..."
  }
}
```

**Notification** (adapter → client, no `id`):
```json
{
  "jsonrpc": "2.0",
  "method": "adapter.diagnostic",
  "params": {}
}
```

---

## Methods

### `adapter.initialize`

Lifecycle signal sent by the client to indicate a session is starting. No configuration is applied.

**Params:** `{}` (none required)

**Result:**
```json
{ "status": "ok" }
```

---

### `adapter.describe`

Returns the adapter's static metadata, configuration schema, default values, and capabilities. Call this after `adapter.initialize` to discover what `adapter.configure` expects.

**Params:** `{}` (none required)

**Result:**
```json
{
  "name": "modbusAdapter",
  "version": "1.0.0",
  "configVersion": 1,
  "schema": { ... },
  "defaults": { ... },
  "capabilities": {
    "supportsHotReload": false,
    "requiresRestartOn": ["connections", "devices"]
  }
}
```

| Field | Description |
| --- | --- |
| `name` | Adapter identifier |
| `version` | Adapter software version |
| `configVersion` | Current config schema version |
| `schema` | JSON Schema–compatible object describing the `config` object accepted by `adapter.configure` |
| `defaults` | Default config values |
| `capabilities` | Feature flags |

Each property in `schema` includes the following additional fields (standard JSON Schema annotations and custom extensions):

| Field | Description |
| --- | --- |
| `title` | Standard JSON Schema annotation. UI-friendly label for the field, suitable for use in form inputs and dialog labels |
| `x-enumLabels` | Custom extension. Present on enum properties only. A string array, parallel to `enum`, giving a UI-friendly display name for each allowed value |

The connection schema uses JSON Schema Draft 7 `if`/`then`/`else` to express type-dependent fields. When `type` equals `"tcp"`, the fields in `then.properties` apply (TCP-specific). Otherwise the fields in `else.properties` apply (serial-specific). A UI can use this to enable or disable the relevant fields based on the selected connection type.

---

### `adapter.configure`

Applies connection and device configuration to the adapter. Must be called before `adapter.start`.

The config object top-level structure will be used for the configuration GUI dialog generation. The GUI uses JSON Schema types to determine layout: `"type": "object"` renders as a single-form dialog, `"type": "array"` renders as a tabbed dialog (one tab per item).

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
        "baudrate": 9600,
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
| `type` | string | yes | `"tcp"` or `"serial"` |
| `timeout` | integer | no | Timeout in milliseconds (default: `1000`) |
| `persistent` | boolean | no | Keep connection open between reads (default: `false`) |

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
| `int32LittleEndian` | boolean | no | Byte order for 32-bit values (default: `false` = big endian) |

**Result:**
```json
{ "status": "ok" }
```

**Errors:**
- `-32602` — Missing `config` key; missing or invalid `general`/`connections`/`devices`; invalid field values

---

### `adapter.start`

Starts Modbus polling with the configuration applied by `adapter.configure`. Must be called after `adapter.configure`.

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

An empty `registers` array is valid and starts polling with no registers configured.

**Result:**
```json
{ "status": "ok" }
```

**Errors:**
- `-32602` — invalid subexpression syntax; unknown type

---

### `adapter.getStatus`

Returns the current poll activity state.

**Params:** `{}` (none required)

**Result:**
```json
{ "active": true }
```

---

### `adapter.readData`

Reads a list of Modbus registers. The adapter starts the read cycle and returns the result when all registers have been sampled.

Only one `readData` request can be in flight at a time. A second request while the first is pending returns an error immediately.

`adapter.start` must have been called before invoking this method.

**Params:** `{}` (none required)

**Result:**
```json
{
  "registers": [
    { "value": 42.0, "valid": true },
    { "value": 0.0,  "valid": false }
  ]
}
```

A register with `"valid": false` could not be read (communication error, timeout, or no device configured). Its `"value"` is `0.0`.

**Errors:**
- `-32000` — Read already in progress

---

### `adapter.shutdown`

Stops all Modbus activity and terminates the adapter process.

**Params:** `{}` (none required)

**Result:**
```json
{ "status": "ok" }
```

The adapter process exits after sending this response.

---

## Notifications

Notifications are sent by the adapter to the client without a corresponding request. They have no `id` field and expect no response.

### `adapter.diagnostic`

_(Reserved for future use.)_ Carries a log or diagnostic message from the adapter.

```json
{
  "jsonrpc": "2.0",
  "method": "adapter.diagnostic",
  "params": {
    "level": "warning",
    "message": "Connection timeout on connection 0"
  }
}
```

| `level` | Meaning |
| --- | --- |
| `"debug"` | Verbose internal trace |
| `"info"` | Informational |
| `"warning"` | Non-fatal issue |

---

## Error Codes

| Code | Name | Description |
| --- | --- | --- |
| `-32700` | Parse error | The message body is not valid JSON |
| `-32600` | Invalid request | Missing `jsonrpc`/`method` fields |
| `-32601` | Method not found | Unknown method name |
| `-32602` | Invalid params | Required fields missing or out of range |
| `-32603` | Internal error | Unexpected server-side error |
| `-32000` | Read in progress | `adapter.readData` called while a previous read has not yet completed |

---

## Session Lifecycle

A typical session follows this sequence:

```text
Client                              Adapter
  |                                    |
  |-- adapter.initialize ------------> |
  |<- { "status": "ok" } ------------ |
  |                                    |
  |-- adapter.configure ------------> |
  |<- { "status": "ok" } ------------ |
  |                                    |
  |-- adapter.start ----------------> |
  |<- { "status": "ok" } ------------ |
  |                                    |
  |-- adapter.getStatus ------------> |
  |<- { "active": true } ------------ |
  |                                    |
  |-- adapter.readData -------------> |
  |                  [Modbus I/O ...]  |
  |<- { "registers": [...] } -------- |
  |                                    |
  |-- adapter.readData -------------> |
  |                  [Modbus I/O ...]  |
  |<- { "registers": [...] } -------- |
  |                                    |
  |-- adapter.shutdown -------------> |
  |<- { "status": "ok" } ------------ |
  |                          [exits]   |
```

If stdin is closed (EOF), the adapter shuts down automatically.

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

Configure connection and device:

```text
Content-Length: 255\r\n
\r\n
{"jsonrpc":"2.0","id":2,"method":"adapter.configure","params":{"config":{"version":1,"general":{},"connections":[{"id":0,"type":"tcp","ip":"192.168.1.100","port":502,"timeout":1000}],"devices":[{"id":1,"connectionId":0,"slaveId":1,"consecutiveMax":64}]}}}
```

Response:

```text
Content-Length: 49\r\n
\r\n
{"id":2,"jsonrpc":"2.0","result":{"status":"ok"}}
```

Start polling:

```text
Content-Length: 90\r\n
\r\n
{"jsonrpc":"2.0","id":3,"method":"adapter.start","params":{"registers":["${40001: 16b}"]}}
```

Response:

```text
Content-Length: 49\r\n
\r\n
{"id":3,"jsonrpc":"2.0","result":{"status":"ok"}}
```

Read request:

```text
Content-Length: 64\r\n
\r\n
{"jsonrpc":"2.0","id":4,"method":"adapter.readData","params":{}}
```

Response (when Modbus read completes):

```text
Content-Length: 77\r\n
\r\n
{"id":4,"jsonrpc":"2.0","result":{"registers":[{"valid":true,"value":1234}]}}
```
