# Configure a Modbus TCP connection

This guide shows how to add and configure a Modbus TCP connection in ModbusScope.

## Steps

1. Go to **Settings > Connection**.
2. If more than one connection slot is available, select the one you want to configure and enable it with the **Enabled** checkbox. Connection 1 is always enabled.
3. Set **Protocol** to `TCP`.
4. Enter the **IP address** of the Modbus TCP slave (IPv4 only, e.g. `192.168.1.10`).
5. Enter the **Port** number (default Modbus port is `502`).
6. Set **Timeout** to the maximum time in milliseconds to wait for a response before giving up.
7. Enable **Persistent connection** if you want the TCP socket to stay open between poll cycles. This reduces connection overhead and is recommended for fast polling.
8. Click **OK**.

**Result:** The connection is saved. You can now link a device to it in **Settings > Device**.

## See also

- [Reference: Connection settings](../reference/connection-settings.md)
- [Configure a Modbus RTU connection](configure-rtu-connection.md)
