# Adapter JSON-RPC 2.0 Protocol Specification

This document specifies the generic, protocol-agnostic contract between the host
application (core) and an adapter child process. It describes the transport, the
set of `adapter.*` methods every adapter must implement, notifications, error
codes, and the session lifecycle.

Concrete payload shapes for a specific adapter (field values in
`adapter.describe`, the config schema consumed by `adapter.configure`,
expression syntax for `adapter.start`, and so on) are defined by that adapter's
implementation spec.

## Overview

The adapter process communicates with a host application (client) via JSON-RPC
2.0 over its standard input and output streams.

The adapter is launched as a child process by the client. The client writes
JSON-RPC requests to the adapter's **stdin** and reads responses and
notifications from its **stdout**.

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

Lifecycle signal sent by the client to indicate a session is starting. No
configuration is applied.

**Params:**

```json
{ "protocolVersion": 2 }
```

`protocolVersion` is the version of this generic contract the client speaks (see
[Protocol version vs. config version](#protocol-version-vs-config-version)
below). The client always sends it; an adapter that does not support the
requested version SHOULD reject the request with an error naming the version(s)
it does support.

**Result:**

```json
{ "status": "ok", "protocolVersion": 2 }
```

`protocolVersion` in the result echoes the version the adapter itself speaks.
The client compares this against the `adapter.describe` response (below) as
the authoritative check — see that section.

---

### `adapter.describe`

Returns the adapter's static metadata, configuration schema, default values, and
capabilities. Call this after `adapter.initialize` to discover what
`adapter.configure` expects.

**Params:** `{}` (none required)

**Result:**

```json
{
  "name": "<adapter-id>",
  "version": "<semver>",
  "configVersion": 1,
  "protocolVersion": 2,
  "schema": { ... },
  "defaults": { ... },
  "capabilities": {
    "supportsHotReload": false,
    "requiresRestartOn": ["connections", "devices"],
    "quality": {
      "flags": ["<flag-id>", ...],
      "protocolNames": { "<flag-id>": "<protocol-mnemonic>", ... }
    }
  }
}
```

See the adapter's implementation spec for a concrete response example.

| Field | Description |
| --- | --- |
| `name` | Adapter identifier |
| `version` | Adapter software version. Release builds carry a plain semver (`"<semver>"`). Debug builds MAY append a pre-release tag and/or `+<build-metadata>` suffix identifying the branch, commit, or other build info. Consumers must not treat `version` as a fixed literal; parse or compare it accordingly. The exact debug format is defined by the adapter's implementation spec. |
| `configVersion` | Current config schema version |
| `protocolVersion` | Version of the generic `adapter.*` contract this adapter speaks — see below |
| `schema` | JSON Schema–compatible object describing the `config` object accepted by `adapter.configure` |
| `defaults` | Default config values |
| `capabilities` | Feature flags; `quality` (optional) declares the data-quality detail flags this adapter may report through `adapter.readData` — see that section |

#### Protocol version vs. config version

`protocolVersion` and `configVersion` version two different things and change
independently:

- `configVersion` versions only the shape of the `config` object accepted by
  `adapter.configure` for *this specific adapter*. It is adapter-defined and
  has no meaning to the generic transport.
- `protocolVersion` versions the generic `adapter.*` contract itself — the
  shape every method in this document produces and expects, most notably
  `adapter.readData`'s result (see below). It is the same integer across every
  adapter, defined by this specification, and MUST match exactly between
  client and adapter.

The client sends `protocolVersion` in `adapter.initialize`'s params and treats
the `protocolVersion` echoed back by `adapter.describe` as authoritative: an
absent value or one that does not exactly equal the version the client speaks
means the adapter is **incompatible**. The client MUST NOT call
`adapter.configure` or `adapter.start` on an incompatible adapter, MUST report
a diagnostic naming the adapter and the found/required versions, and MUST
otherwise leave the adapter unusable (no data is read) — a mismatch here means
the two sides could disagree on the meaning of `adapter.readData`'s result, so
guessing is not an option. A stale adapter binary silently read under the old
shape, with its `"valid": false` misread as a good `0.0`, is exactly the
failure mode this guard exists to prevent.

#### Structural vs. enforced limits

`schema` and `capabilities` can each express a maximum for the same concept,
but they mean different things:

- A JSON Schema constraint on a `schema` property (e.g. `maxItems` on an
  array-valued property such as `devices`) is a **fixed, structural** upper
  bound — the largest value `adapter.configure` could structurally ever
  accept, independent of runtime state such as licensing.
- A matching entry in `capabilities` (e.g. a `max<Noun>` key, such as
  `maxDevices`) is the **live, currently-enforced** limit. It MAY be smaller
  than the schema's structural bound, and MAY change between `adapter.describe`
  calls — for example, when a license state changes.

When both exist for the same concept, `capabilities` is authoritative for
what `adapter.configure` will actually accept right now. Clients that
validate configuration, warn on over-limit input, or truncate configuration
before sending it must consult `capabilities`, not just `schema` — relying on
`schema` alone under-reports the effective limit whenever the two diverge.
See the adapter's implementation spec for which capability keys, if any,
mirror a schema limit.

Each property in `schema` MAY include the following additional fields (standard
JSON Schema annotations and custom extensions):

| Field | Description |
| --- | --- |
| `title` | Standard JSON Schema annotation. UI-friendly label for the field, suitable for use in form inputs and dialog labels |
| `x-enumLabels` | Custom extension. Present on enum properties only. A string array, parallel to `enum`, giving a UI-friendly display name for each allowed value |

Schemas MAY use JSON Schema Draft 7 `if`/`then`/`else` to express type-dependent
fields: when a discriminator field matches a value, the fields in
`then.properties` apply; otherwise the fields in `else.properties` apply. A UI
can use this to enable or disable the relevant fields based on the selected
value.

---

### `adapter.configure`

Applies configuration to the adapter. Must be called before `adapter.start`.

The `config` object's shape must conform to the `schema` returned by
`adapter.describe`. The top-level structure is also used for configuration GUI
dialog generation: `"type": "object"` renders as a single-form dialog, `"type":
"array"` renders as a tabbed dialog (one tab per item).

**Params:**

```json
{
  "config": { ... }
}
```

See the adapter's implementation spec for the concrete `config` shape and field
semantics.

**Result:**

```json
{ "status": "ok" }
```

**Errors:**

- `-32602` — Missing `config` key; missing or invalid required fields; invalid
  field values

---

### `adapter.start`

Starts data-source activity (polling, subscriptions, etc.) using the
configuration applied by `adapter.configure`. Must be called after
`adapter.configure`.

**Params:**

```json
{
  "dataPoints": ["<expression>", "<expression>", ...]
}
```

Each element of `dataPoints` is an adapter-specific expression string identifying
a data point to observe. The syntax of expressions is defined by the adapter's
implementation spec (see `adapter.expressionHelp` for the HTML rendering and
`adapter.dataPointSchema` for the structured form).

An empty `dataPoints` array is valid and starts activity with no data points
configured.

**Result:**

```json
{ "status": "ok" }
```

**Errors:**

- `-32602` — invalid expression syntax; unknown type

---

### `adapter.dataPointSchema`

Returns the schema for data point expressions — what fields make up a data point
address, how they should be rendered in the UI, and available data types. Call
this after `adapter.describe` to discover how to build the data point input UI.

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

See the adapter's implementation spec for the concrete `addressSchema`,
`dataTypes`, and `defaultDataType` values.

| Field | Description |
| --- | --- |
| `addressSchema` | JSON Schema describing the address input fields. The core renders this with `SchemaFormWidget` |
| `dataTypes` | Array of available data types. Each entry has `id` (used in expression strings) and `label` (UI display) |
| `defaultDataType` | The `id` of the type to pre-select in the UI |

The `addressSchema` follows standard JSON Schema conventions. The core
application uses it to dynamically generate the address input portion of the
data point dialog, so it must accurately describe all required fields and their
constraints. The `dataType` property within `addressSchema` has no `enum`
constraint; the available values are supplied by the top-level `dataTypes`
array, and `defaultDataType` indicates which value to pre-select.

---

### `adapter.describeDataPoint`

Parses a data point expression into structured fields and returns a
human-readable description. Used by the core to display data point details in
tables and tooltips without understanding protocol-specific address formats.

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

Validates a single data point expression string without starting polling. Used
for real-time validation feedback in the data point input dialog.

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

Constructs a data point expression string from its component parts. The core
calls this after the user fills in the address form and selects a data type and
device, so expression syntax stays entirely within the adapter.

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

Returns a static HTML fragment describing the adapter-specific data point
addressing syntax. The core prepends its own generic expression syntax
explanation (operators, number formats) and displays the combined HTML in the
expression editor info panel, so the adapter only needs to document the
syntax it owns.

**Params:** `{}` (none required)

**Result:**

```json
{ "helpText": "<p>...</p>" }
```

| Field | Type | Description |
| --- | --- | --- |
| `helpText` | string | HTML fragment (no `<html>`/`<body>` wrapper) suitable for embedding in a rich-text label |

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

Reads the current value of each configured data point. The adapter starts the
read cycle and returns the result when all data points have been sampled.

Only one `readData` request can be in flight at a time. A second request while
the first is pending returns an error immediately.

`adapter.start` must have been called before invoking this method.

**Params:** `{}` (none required)

**Result:**

```json
{
  "dataPoints": [
    { "value": 42.0 },
    { "value": 12.5, "state": "degraded", "flags": ["substituted"] },
    { "state": "invalid", "flags": ["oldData"] },
    { "state": "invalid" },
    { "state": "noValue" }
  ]
}
```

The `dataPoints` array has the same length and order as the `dataPoints` array
passed to `adapter.start`. Each entry carries a `state`, an optional `value`,
and an optional `flags` array:

| Field | Rule |
| --- | --- |
| `value` | A number, present if and only if the point is usable (`state` is `good` or `degraded`). Never present, never a fabricated `0.0`, for `invalid` or `noValue`. A client MUST treat a `good` or `degraded` point (or a `dataPoints` entry that is not an object) that has no numeric `value` as `invalid` (plus one diagnostic per session) rather than as a value of `0.0`. |
| `state` | One of `good`, `degraded`, `invalid`, `noValue`. **Omitted means `good`** — a producer MAY always write it explicitly, but the common case need not spend the bytes. A client MUST accept both forms and MUST reject an unrecognised value by treating the point as `invalid` (plus one diagnostic) — it MUST NOT fall back to `good`. |
| `flags` | Optional array of adapter-defined detail-flag ids (e.g. `"substituted"`), further qualifying *why* a point is `degraded` or `invalid`. Omitted or empty means no flags. A `good` point that carries flags is contradictory; a client SHOULD treat it as `degraded`. A client MUST ignore any flag id it does not recognise (collecting unrecognised ids into at most one diagnostic per session) rather than reject the point — this is what lets an adapter add a new flag without a `protocolVersion` bump. The set of flag ids a given adapter may ever send is declared in `adapter.describe`'s `capabilities.quality` (see that section); an adapter that declares no `quality` capability sends no `flags` at all. |

State meanings:

- **`good`** — a normal measurement, no caveats.
- **`degraded`** — a real, usable measurement the source flagged in some way (see `flags` for detail); the client may use `value` but should be able to indicate to the user that it is not a plain good reading.
- **`invalid`** — the source says this value must not be used. Covers both a communication error and the source explicitly marking the value as invalid at the point of origin.
- **`noValue`** — no measurement has been received yet for this point (for example, immediately after `adapter.start`, before the first read cycle has populated it). Distinct from `invalid`: nothing is wrong, this data point's first value simply has not arrived.

**Errors:**

- `-32000` — Read already in progress (a previous `readData` has not yet
  completed)
- `-32000` — Read timed out (data-source I/O did not complete within the timeout
  window)

---

### `adapter.stop`

Stops data-source activity (polling, subscriptions, etc.) without terminating
the adapter process. The adapter returns to the state it was in after
`adapter.configure`, ready to accept a new `adapter.configure` + `adapter.start`
sequence.

Use `adapter.stop` to pause between sessions while keeping the adapter process
alive. Use `adapter.shutdown` only when the adapter process should exit.

**Params:** `{}` (none required)

**Result:**

```json
{ "status": "ok" }
```

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

Notifications are sent by the adapter to the client without a corresponding
request. They have no `id` field and expect no response.

### `adapter.diagnostic`

Carries a log or diagnostic message from the adapter. Emitted for every Qt log
message (`qDebug`, `qInfo`, `qWarning`, `qCritical`, `qFatal`) produced during
adapter operation.

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
| `-32000` | Server error | `adapter.readData` called while a previous read has not yet completed, or the read timed out |

---

## Session Lifecycle

### Initialization

The adapter process is started once and initialized with `adapter.initialize`
and `adapter.describe`. After `adapter.describe` the adapter is in
AWAITING_CONFIG state and ready to accept configuration.

```text
Client                                            Adapter
  |                                                  |
  |-- adapter.initialize {"protocolVersion":2} ---> |
  |<- { "status": "ok", "protocolVersion": 2 } ----- |
  |                                                  |
  |-- adapter.describe ---------------------------> |
  |<- { "name": ..., "protocolVersion": 2, ... } --- |
  |    (client checks protocolVersion matches before proceeding)
  |                                        [AWAITING_CONFIG]
```

### Session (repeatable)

Once initialized, the client may run any number of sessions without restarting
the adapter process. Each session starts with `adapter.configure` to ensure
settings are in sync, followed by `adapter.start` to begin data-source activity.

```text
  |-- adapter.configure ------------> |
  |<- { "status": "ok" } ------------ |
  |                                    |
  |-- adapter.start ----------------> |
  |<- { "status": "ok" } ------------ |
  |                          [ACTIVE]  |
  |-- adapter.readData -------------> |
  |                  [I/O ...]         |
  |<- { "dataPoints": [...] } ------- |
  |    (readData loop repeats)         |
  |                                    |
  |-- adapter.stop -----------------> |
  |<- { "status": "ok" } ------------ |
  |                          [AWAITING_CONFIG]
  |    (next session: back to configure)
```

### Teardown

When the adapter process should exit, close stdin (EOF) or send
`adapter.shutdown`. The adapter shuts down automatically on EOF.

```text
  |-- adapter.shutdown -------------> |
  |<- { "status": "ok" } ------------ |
  |                          [exits]   |
```

If stdin is closed (EOF), the adapter shuts down automatically.
