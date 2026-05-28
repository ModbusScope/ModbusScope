# Polling and sample rate

Understanding what determines the actual sample rate helps you configure ModbusScope to get the data resolution you need.

## The poll cycle

ModbusScope uses a timer to trigger poll cycles. Each cycle reads all active registers, appends the results to the graph, and then waits for the next timer tick. The **Poll interval** setting in Log settings is the target period for this timer — but it is a lower bound, not a guarantee.

The actual interval is `max(poll_interval, time_to_read_all_registers)`. If reading all registers takes longer than the configured interval, cycles stretch to accommodate it.

## What affects how long a read takes

**Number of registers.** Each additional register is at minimum one more value to request. More registers, more time.

**Gaps between register addresses.** Modbus allows reading a block of consecutive registers in a single request. ModbusScope groups consecutive addresses into batches automatically. When there is a gap between addresses, the batch must end and a new request must start for the next group. Each additional request adds the Modbus end-of-frame gap — a mandatory silence period on the bus — which is measured in milliseconds on slower serial speeds.

**Device response time.** The device must process the request and reply within the configured timeout. Slow devices or congested networks add latency to every request.

**Number of devices.** Reads to different devices cannot be merged. A session with two devices requires at least two separate exchanges, even if the registers are otherwise identical.

**TCP connection overhead.** Without persistent connection enabled, ModbusScope opens and closes a TCP socket each poll cycle. The TCP handshake adds round-trip time. Enabling persistent connection avoids this.

## Getting a faster rate

The most effective actions, roughly in order of impact:

1. Remove registers you do not need.
2. Arrange register addresses to be consecutive (requires device firmware cooperation).
3. Enable persistent connection for TCP devices.
4. Increase max consecutive registers in Device settings if the device supports it.
5. Lower the poll interval — but only after the steps above, because the transport latency sets a floor regardless of this setting.

## See also

- [Optimize poll rate (how-to)](../how-to/optimize-poll-rate.md)
- [Reference: Log settings](../reference/log-settings.md)
- [Reference: Device settings](../reference/device-settings.md)
