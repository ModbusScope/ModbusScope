# Connections and devices

ModbusScope separates the concepts of a connection and a device. Users new to the application sometimes expect to configure everything in one dialog, so this distinction is worth understanding before you set up your first session.

## What a connection is

A connection describes the transport: how bytes move between the computer running ModbusScope and the physical bus. This is either a TCP socket (an IP address and port) or a serial port (a COM port name with its baud rate, parity, and framing parameters). ModbusScope opens and manages this transport for you.

## What a device is

A device describes a Modbus slave sitting on that transport. It carries the Modbus-protocol parameters: slave ID, timeout, the maximum number of registers to request in one packet, and the byte order for 32-bit values. Each device is linked to exactly one connection.

## Why they are separate

A single physical bus can carry multiple Modbus slaves. On an RS-485 RTU bus, for example, a dozen sensors may all share the same serial port but have different slave IDs. In ModbusScope you model this by creating one connection (the serial port) and one device per slave. All devices on that bus share the connection but each has its own slave ID.

The same applies to TCP: a TCP gateway that bridges a serial bus typically presents each slave on the same IP address and port but with a different Modbus unit ID. One connection, multiple devices.

This structure also lets you mix transports in the same logging session. You could log a TCP device and an RTU device at the same time by creating two connections and one device linked to each.

## How registers tie in

When you add a register, you specify which device it belongs to using the `@DEVICE` part of the expression syntax. ModbusScope uses the device's connection and Modbus parameters when reading that register. Registers on different devices can be polled in the same session; ModbusScope schedules the reads across all active devices within each poll cycle.

## See also

- [Reference: Connection settings](../reference/connection-settings.md)
- [Reference: Device settings](../reference/device-settings.md)
- [Configure a TCP connection](../how-to/configure-tcp-connection.md)
