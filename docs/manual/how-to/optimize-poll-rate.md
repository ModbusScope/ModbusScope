# Optimize the poll rate

This guide shows how to reduce the logging interval when samples are arriving too slowly.

## Steps

1. Go to **Settings > Log**.
2. Lower the **Poll interval** value (the default is `250 ms`).
3. Click **OK**.
4. Start logging and observe whether the actual sample rate meets the reduced interval.

If the interval is still slower than expected, the bottleneck is the Modbus protocol, not the poll timer. Apply the steps below.

## Reduce Modbus overhead

**Remove unused registers.**
Each additional register adds at least one Modbus request. Remove registers you do not need.

**Group registers at consecutive addresses.**
ModbusScope reads consecutive addresses in a single packet. A gap between addresses forces a separate request. If your device allows it, arrange the registers you poll so their addresses are adjacent (e.g. `40001`, `40002`, `40003` rather than `40001`, `40010`, `40050`).

**Increase max consecutive registers per device.**
If your device can handle larger read requests, increase **Max consecutive registers** in **Settings > Device**. This allows ModbusScope to fetch more registers per packet when addresses are grouped.

**Enable persistent connection (TCP only).**
When **Persistent connection** is off, ModbusScope opens and closes the TCP socket each poll cycle. Enabling it removes this overhead.

**Result:** After applying these changes, the effective sample rate should be closer to the configured poll interval.

## See also

- [Reference: Log settings](../reference/log-settings.md)
- [Reference: Device settings](../reference/device-settings.md)
- [Explanation: Polling and sample rate](../explanation/polling-and-sample-rate.md)
