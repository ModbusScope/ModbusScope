# Adapter JSON-RPC Sequence Diagram

This document describes the full JSON-RPC 2.0 message exchange between the host application (`AdapterClient`) and the adapter process (`ModbusAdapter`) over Content-Length–framed stdio.

---

## Normal Session (Happy Path)

```
Client (AdapterClient)                    Adapter (ModbusAdapter)
         |                                          |
[IDLE]   |                                          |
         | QProcess::start(adapterPath)             |
         |                                          |
         |-- Content-Length: 66                     |
         |   {"jsonrpc":"2.0","id":1,               |
         |    "method":"adapter.initialize",        |
         |    "params":{}}                         -->  InitializeHandler
[INIT]   |                                          |    returns {"status":"ok"}
         |<- {"id":1,"jsonrpc":"2.0",               |
         |   "result":{"status":"ok"}}              |
         |                                          |
         |-- {"id":2,                               |
         |    "method":"adapter.describe",          |
         |    "params":{}}                         -->  DescribeHandler
[DESC]   |                                          |    returns name/version/schema/defaults/caps
         |<- {"id":2,"result":{                     |
         |    "name":"modbusAdapter",               |
         |    "version":"1.0.0",                    |
         |    "configVersion":1,                    |
         |    "schema":{...},                       |
         |    "defaults":{...},                     |
         |    "capabilities":{                      |
         |      "supportsHotReload":false,          |
         |      "requiresRestartOn":               |
         |        ["connections","devices"]}}}      |
         |                                          |
[AWAIT]  | AdapterManager.onDescribeResult()        |
         |   -> updateAdapterFromDescribe()         |
         |   -> requestDataPointSchema() [auto]     |
         |                                          |
         |-- {"id":3,                               |
         |    "method":"adapter.dataPointSchema",   |
         |    "params":{}}                         -->  DataPointSchemaHandler
         |<- {"id":3,"result":{                     |
         |    "addressSchema":{...},                |
         |    "dataTypes":[...],                    |
         |    "defaultDataType":"16b"}}             |
         |   -> setAdapterDataPointSchema()         |
         |                                          |
         | --- optional UI aux calls (any order) -- |
         |                                          |
         |-- {"id":4,                               |
         |    "method":"adapter.buildExpression",   |
         |    "params":{"fields":{...},             |
         |    "dataType":"f32b","deviceId":2}}      -->  BuildExpressionHandler
         |<- {"id":4,"result":                      |
         |    {"expression":"${h0@2:f32b}"}}        |
         |                                          |
         |-- {"id":5,                               |
         |    "method":"adapter.validateDataPoint", |
         |    "params":                             |
         |    {"expression":"${40001:16b}"}}       -->  ValidateDataPointHandler
         |<- {"id":5,"result":{"valid":true}}       |
         |                                          |
         |-- {"id":6,                               |
         |    "method":"adapter.describeDataPoint", |
         |    "params":                             |
         |    {"expression":"${40001:16b}"}}       -->  DescribeDataPointHandler
         |<- {"id":6,"result":{                     |
         |    "valid":true,                         |
         |    "fields":{"objectType":              |
         |      "holding register",                 |
         |      "address":0,"deviceId":1,           |
         |      "dataType":"16b"},                  |
         |    "description":"holding register..."}} |
         |                                          |
         |-- {"id":7,                               |
         |    "method":"adapter.expressionHelp",    |
         |    "params":{}}                         -->  ExpressionHelpHandler
         |<- {"id":7,"result":                      |
         |    {"helpText":"<html>...</html>"}}      |
         |                                          |
         | --- provideConfig(config, expressions) -- |
         |                                          |
         |-- {"id":8,                               |
         |    "method":"adapter.configure",         |
         |    "params":{"config":{                  |
         |      "version":1,"general":{},           |
         |      "connections":[{"id":0,            |
         |        "type":"tcp",                     |
         |        "ip":"192.168.1.100",             |
         |        "port":502,"timeout":1000}],      |
         |      "devices":[{"id":1,                |
         |        "connectionId":0,                 |
         |        "slaveId":1,                      |
         |        "consecutiveMax":64}]}}}          -->  ConfigureHandler
[CONFIG] |                                          |    phase 1: validate all
         |                                          |    phase 2: commit to model
         |<- {"id":8,"result":{"status":"ok"}}      |
         |                                          |
         |-- {"id":9,                               |
         |    "method":"adapter.start",             |
         |    "params":{"registers":              |
         |      ["${40001:16b}","${h0@2:f32b}"]}}  -->  StartHandler
[START]  |                                          |    -> ModbusPoll.startCommunication()
         |                                          |    -> _bPollActive = true
         |<- {"id":9,"result":{"status":"ok"}}      |
         |                                          |
[ACTIVE] |                                          |
         |-- {"id":10,                              |
         |    "method":"adapter.getStatus",         |
         |    "params":{}}                         -->  GetStatusHandler
         |<- {"id":10,"result":{"active":true}}     |    -> ModbusPoll.isActive()
         |                                          |
         |-- {"id":11,                              |
         |    "method":"adapter.readData",          |
         |    "params":{}}                         -->  ReadDataHandler [DEFERRED]
         |                  [Modbus TCP/Serial I/O] |    -> triggerRegisterRead()
         |<- {"id":11,"result":{"registers":[       |    -> registerDataReady signal
         |    {"value":1234,"valid":true},          |    -> emit responseReady()
         |    {"value":0.0,"valid":false}]}}        |
         |                                          |
         |   (readData loop repeats as needed)      |
         |                                          |
         | --- stopSession() ---                    |
         |                                          |
         |-- {"id":12,                              |
         |    "method":"adapter.shutdown",          |
         |    "params":{}}                         -->  ShutdownHandler
[STOP]   |                                          |    -> stopCommunication()
         |<- {"id":12,"result":{"status":"ok"}}     |    -> _bPollActive = false
         |  _pProcess->stop()                       |    -> emit shutdownRequested()
         |  [close write channel, kill timer 3s]   |    -> QApplication::quit()
         |                  [process exits]         |
[IDLE]   |<- onProcessFinished()                   |
         |   emit sessionStopped()                 |
```

---

## Adapter Notification (any time)

```
Adapter (any handler)                     Client (AdapterClient)
         |                                          |
         |-- {"jsonrpc":"2.0",                      |
         |    "method":"adapter.diagnostic",        |
         |    "params":{                            |
         |      "level":"warning",                  |
         |      "message":"Connection timeout..."}} |
         |                                         -->  onNotificationReceived()
         |                                               emit diagnosticReceived(level, msg)
         |                                               -> AdapterManager.onAdapterDiagnostic()
         |                                               -> qCWarning(scopeAdapter)
```

---

## Error Paths

### Handshake timeout (10 s, any in-progress state)

```
[any]    |   _handshakeTimer fires (10 000 ms)      |
         |   -> onHandshakeTimeout()                |
         |   -> _pProcess->stop()                   |
         |   emit sessionError("Adapter handshake timed out")
[IDLE]   |
```

### Adapter process crash (unexpected exit)

```
[ACTIVE] |                  [process exits]         |
         |<- onProcessFinished() [state != STOPPING]|
         |   emit sessionError("Adapter process exited unexpectedly")
[IDLE]   |
```

### RPC error on a lifecycle method

```
[CONFIG] |-- adapter.configure -----------------> |
         |<- {"id":N,"error":{                     |
         |    "code":-32602,                        |
         |    "message":"Invalid params: ..."}}     |
         |   -> onErrorReceived()                   |
         |   -> _state = IDLE                       |
         |   -> _pProcess->stop()                   |
         |   emit sessionError("Adapter error on adapter.configure: ...")
[IDLE]   |
```

### readData timeout (adapter-side, 5 s)

```
[ACTIVE] |-- adapter.readData -----------------> |
         |        [Modbus I/O stalls > 5 s]        |
         |<- {"id":N,"error":{                     |
         |    "code":-32000,                        |
         |    "message":"Read timed out"}}          |
         |   -> onErrorReceived()                   |
         |   *** treated as FATAL session error *** |
         |   emit sessionError("Adapter error on adapter.readData: Read timed out")
[IDLE]   |
```

> **Known issue:** a Modbus read timeout should be a recoverable per-read failure, not a
> session-level error. See GitHub issue for the fix in `AdapterClient::onErrorReceived()`.

### stopSession() called before ACTIVE (no adapter.shutdown sent)

```
[INIT/   |   stopSession()                          |
DESC/    |   -> _state = STOPPING                   |
AWAIT/   |   -> _pProcess->stop()                   |
CONFIG]  |   [close write channel, kill timer 3s]   |
         |                  [process exits]          |
[IDLE]   |<- onProcessFinished()                    |
         |   emit sessionStopped()                  |
```

---

## State Machine Summary

| State | Entered from | Key action | Leaves to |
|---|---|---|---|
| `IDLE` | startup / `sessionStopped` / error | — | `INITIALIZING` |
| `INITIALIZING` | `prepareAdapter()` | send `adapter.initialize`, start 10 s timer | `DESCRIBING` |
| `DESCRIBING` | `initialize` response | send `adapter.describe` | `AWAITING_CONFIG` |
| `AWAITING_CONFIG` | `describe` response | emit `describeResult()`, stop timer | `CONFIGURING` |
| `CONFIGURING` | `provideConfig()` | send `adapter.configure`, start 10 s timer | `STARTING` |
| `STARTING` | `configure` response | send `adapter.start` | `ACTIVE` |
| `ACTIVE` | `start` response | emit `sessionStarted()`, stop timer | `STOPPING` |
| `STOPPING` | `stopSession()` from ACTIVE/STARTING | send `adapter.shutdown`, start 10 s timer | `IDLE` |
| `STOPPING` | `stopSession()` from any other state | `_pProcess->stop()` directly | `IDLE` |
