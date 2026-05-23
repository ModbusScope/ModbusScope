# Diagnostic logs

ModbusScope records a log entry for every register read it attempts. This page explains what the log categories mean and how to use them to diagnose problems.

## Opening the logs

Go to **Help > Diagnostic logs...**. The window shows a live list of log entries. Entries are colour-coded by category, and you can filter categories using the checkboxes at the top.

![Diagnostic logs window](<../_static/user_manual/diagnostic_logs.png>)

## Log categories

**Info** entries record session lifecycle events: when ModbusScope starts a logging session, which version of the application is running, and the operating system. These give context when sharing logs with someone else.

**Communication error** entries appear when the transport fails before a Modbus response arrives. Common causes: wrong IP address, wrong serial port, device powered off, cable fault, or a firewall blocking the TCP connection. The error appears immediately on the first cycle that fails.

**Timeout** entries appear when the connection is established but the device does not respond within the configured timeout period. This usually means the slave ID is wrong, the timeout is set too short for a slow device, or the device is overloaded. Increasing the timeout in Device settings often resolves this.

**Modbus exception** entries appear when the device responds but signals an error at the protocol level. Common exception codes:

| Code | Name | Typical cause |
|---|---|---|
| 1 | Illegal function | The device does not support the requested function code (e.g. coil reads on a device that has none) |
| 2 | Illegal data address | The register address does not exist on the device |
| 3 | Illegal data value | The request is malformed (e.g. requesting more registers than the protocol allows) |

**Debug** entries contain detailed per-read information: the exact request sent, the response received, and timing. This level of detail is off by default because it generates a large volume of entries and slows down the log window. Enable it only when the other categories are not enough to diagnose an issue.

## Exporting logs

Click **Export Logs** to save all visible log entries to a text file. This is useful when reporting a problem, as it captures the exact sequence of events during a session.

Right-clicking an entry lets you copy it individually to the clipboard.

## See also

- [Optimize poll rate](../how-to/optimize-poll-rate.md)
- [Reference: Connection settings](../reference/connection-settings.md)
- [Reference: Device settings](../reference/device-settings.md)
