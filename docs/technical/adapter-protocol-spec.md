# Adapter JSON-RPC 2.0 Protocol Specification

This document specifies the generic, protocol-agnostic contract between the host application (core) and an adapter child process. It describes the transport, the set of `adapter.*` methods every adapter must implement, notifications, error codes, and the session lifecycle.

Concrete payload shapes for a specific adapter (field values in `adapter.describe`, the config schema consumed by `adapter.configure`, expression syntax for `adapter.start`, and so on) are defined by that adapter's implementation spec.

## Overview

The adapter process communicates with a host application (client) via JSON-RPC 2.0 over its standard input and output streams.

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
  "name": "<adapter-id>",
  "version": "<semver>",
  "configVersion": 1,
  "schema": { ... },
  "defaults": { ... },
  "capabilities": {
    "supportsHotReload": false,
    "requiresRestartOn": ["connections", "devices"]
  }
}
```

See the adapter's implementation spec for a concrete response example.

| Field | Description |
| --- | --- |
| `name` | Adapter identifier |
| `version` | Adapter software version. Release builds carry a plain semver (`"<semver>"`). Debug builds MAY append a pre-release tag and/or `+<build-metadata>` suffix identifying the branch, commit, or other build info. Consumers must not treat `version` as a fixed literal; parse or compare it accordingly. The exact debug format is defined by the adapter's implementation spec. |
| `configVersion` | Current config schema version |
| `schema` | JSON Schema–compatible object describing the `config` object accepted by `adapter.configure` |
| `defaults` | Default config values |
| `capabilities` | Feature flags |

Each property in `schema` MAY include the following additional fields (standard JSON Schema annotations and custom extensions):

| Field | Description |
| --- | --- |
| `title` | Standard JSON Schema annotation. UI-friendly label for the field, suitable for use in form inputs and dialog labels |
| `x-enumLabels` | Custom extension. Present on enum properties only. A string array, parallel to `enum`, giving a UI-friendly display name for each allowed value |

Schemas MAY use JSON Schema Draft 7 `if`/`then`/`else` to express type-dependent fields: when a discriminator field matches a value, the fields in `then.properties` apply; otherwise the fields in `else.properties` apply. A UI can use this to enable or disable the relevant fields based on the selected value.

---

### `adapter.configure`

Applies configuration to the adapter. Must be called before `adapter.start`.

The `config` object's shape must conform to the `schema` returned by `adapter.describe`. The top-level structure is also used for configuration GUI dialog generation: `"type": "object"` renders as a single-form dialog, `"type": "array"` renders as a tabbed dialog (one tab per item).

**Params:**
```json
{
  "config": { ... }
}
```

See the adapter's implementation spec for the concrete `config` shape and field semantics.

**Result:**
```json
{ "status": "ok" }
```

**Errors:**
- `-32602` — Missing `config` key; missing or invalid required fields; invalid field values

---

### `adapter.start`

Starts data-source activity (polling, subscriptions, etc.) using the configuration applied by `adapter.configure`. Must be called after `adapter.configure`.

**Params:**

```json
{
  "registers": ["<expression>", "<expression>", ...]
}
```

Each element of `registers` is an adapter-specific expression string identifying a data point to observe. The syntax of expressions is defined by the adapter's implementation spec (see `adapter.expressionHelp` for the HTML rendering and `adapter.dataPointSchema` for the structured form).

An empty `registers` array is valid and starts activity with no data points configured.

**Result:**
```json
{ "status": "ok" }
```

**Errors:**
- `-32602` — invalid expression syntax; unknown type

---

### `adapter.dataPointSchema`

Returns the schema for data point expressions — what fields make up a data point address, how they should be rendered in the UI, and available data types. Call this after `adapter.describe` to discover how to build the data point input UI.

**Params:** `{}` (none required)

**Result:**
```json
{
  "addressSchema": { ... },
  "dataTypes": [
    { "id": "<type-id>", "label": "<UI label>" }
  ],
  "defaultDataType": "<type-id>"
}
```

See the adapter's implementation spec for the concrete `addressSchema`, `dataTypes`, and `defaultDataType` values.

| Field | Description |
| --- | --- |
| `addressSchema` | JSON Schema describing the address input fields. The core renders this with `SchemaFormWidget` |
| `dataTypes` | Array of available data types. Each entry has `id` (used in expression strings) and `label` (UI display) |
| `defaultDataType` | The `id` of the type to pre-select in the UI |

The `addressSchema` follows standard JSON Schema conventions. The core application uses it to dynamically generate the address input portion of the data point dialog, so it must accurately describe all required fields and their constraints. The `dataType` property within `addressSchema` has no `enum` constraint; the available values are supplied by the top-level `dataTypes` array, and `defaultDataType` indicates which value to pre-select.

---

### `adapter.describeDataPoint`

Parses a data point expression into structured fields and returns a human-readable description. Used by the core to display data point details in tables and tooltips without understanding protocol-specific address formats.

**Params:**
```json
{
  "expression": "<expression>"
}
```

**Result (valid):**
```json
{
  "valid": true,
  "fields": { ... },
  "description": "<human-readable>"
}
```

**Result (invalid):**
```json
{
  "valid": false,
  "error": "<message>"
}
```

| Field | Description |
| --- | --- |
| `valid` | Whether the expression is syntactically and semantically valid |
| `fields` | Structured parsed fields — protocol-specific, but the core treats them as opaque display data |
| `description` | Human-readable description for display in tables, tooltips, and logs |
| `error` | Human-readable error message when `valid` is false |

**Errors:**
- `-32602` — Missing `expression` field

---

### `adapter.validateDataPoint`

Validates a single data point expression string without starting polling. Used for real-time validation feedback in the data point input dialog.

**Params:**
```json
{
  "expression": "<expression>"
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
  "error": "<message>"
}
```

| Field | Description |
| --- | --- |
| `valid` | Whether the expression is valid |
| `error` | Human-readable error message when `valid` is false |

**Errors:**
- `-32602` — Missing `expression` field

---

### `adapter.buildExpression`

Constructs a data point expression string from its component parts. The core calls this after the user fills in the address form and selects a data type and device, so expression syntax stays entirely within the adapter.

**Params:**

```json
{
  "fields": { ... },
  "dataType": "<type-id>",
  "deviceId": 2
}
```

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| `fields` | object | yes | Address field values as returned by the data point schema form (structure matches `addressSchema` from `adapter.dataPointSchema`) |
| `dataType` | string | no | Data type identifier. Omit to use the adapter default |
| `deviceId` | integer | no | Device identifier from `adapter.configure`. Omit to use the adapter default |

**Result:**

```json
{ "expression": "<expression>" }
```

**Errors:**

- `-32602` — Missing or invalid `fields`; unknown `dataType`

---

### `adapter.expressionHelp`

Returns static HTML help text describing the data point expression syntax. The core displays this in the expression editor info panel so the explanation stays co-located with the adapter that owns the syntax.

**Params:** `{}` (none required)

**Result:**

```json
{ "helpText": "<html>...</html>" }
```

| Field | Type | Description |
| --- | --- | --- |
| `helpText` | string | HTML string suitable for display in a rich-text label |

---

### `adapter.getStatus`

Returns the current activity state.

**Params:** `{}` (none required)

**Result:**
```json
{ "active": true }
```

---

### `adapter.readData`

Reads the current value of each configured data point. The adapter starts the read cycle and returns the result when all data points have been sampled.

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

The `registers` array has the same length and order as the `registers` array passed to `adapter.start`. Each entry has a numeric `value` (double) and a `valid` flag. A register with `"valid": false` could not be read (communication error, timeout, or no device configured). Its `"value"` is `0.0`.

**Errors:**
- `-32000` — Read already in progress

---

### `adapter.shutdown`

Stops all activity and terminates the adapter process.

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

Carries a log or diagnostic message from the adapter. Emitted for every Qt log message (`qDebug`, `qInfo`, `qWarning`, `qCritical`, `qFatal`) produced during adapter operation.

```json
{
  "jsonrpc": "2.0",
  "method": "adapter.diagnostic",
  "params": {
    "level": "warning",
    "message": "..."
  }
}
```

| `level` | Meaning |
| --- | --- |
| `"debug"` | Verbose internal trace |
| `"info"` | Informational |
| `"warning"` | Non-fatal issue |
| `"error"` | Critical or fatal error |

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
  |                  [I/O ...]         |
  |<- { "registers": [...] } -------- |
  |                                    |
  |-- adapter.readData -------------> |
  |                  [I/O ...]         |
  |<- { "registers": [...] } -------- |
  |                                    |
  |-- adapter.shutdown -------------> |
  |<- { "status": "ok" } ------------ |
  |                          [exits]   |
```

If stdin is closed (EOF), the adapter shuts down automatically.
