# Configure a Modbus RTU connection

This guide shows how to add and configure a Modbus RTU (serial) connection in ModbusScope.

## Steps

1. Go to **Settings > Connection**.
2. If more than one connection slot is available, select the one you want to configure and enable it with the **Enabled** checkbox. Connection 1 is always enabled.
3. Set **Protocol** to `RTU`.
4. Set **Serial port** to the port name on your system (e.g. `COM3` on Windows, `/dev/ttyS0` on Linux).
5. Set **Baud rate** to match the device (common values: `9600`, `19200`, `115200`).
6. Set **Parity** to match the device (`None`, `Even`, or `Odd`).
7. Set **Data bits** to match the device (typically `8`).
8. Set **Stop bits** to match the device (typically `1` or `2`).
9. Set **Timeout** to the maximum time in milliseconds to wait for a response.
10. Click **OK**.

**Result:** The connection is saved. You can now link a device to it in **Settings > Device**.

> All serial parameters (baud rate, parity, data bits, stop bits) must match the settings on the physical Modbus bus. A mismatch will prevent communication.

## See also

- [Reference: Connection settings](../reference/connection-settings.md)
- [Configure a Modbus TCP connection](configure-tcp-connection.md)
