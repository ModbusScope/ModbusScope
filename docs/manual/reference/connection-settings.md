# Connection settings reference

Open via **Settings > Connection**.

Each connection slot can be configured independently. Connection 1 is always enabled; additional connections can be enabled or disabled.

## Common settings

| Setting | Type | Default | Description |
| --- | --- | --- | --- |
| Enabled | Checkbox | On (conn. 1), Off (others) | Enables or disables this connection. Cannot be disabled for connection 1 |
| Protocol | `TCP` / `RTU` | `TCP` | Transport type |
| Timeout | Integer (ms) | `1000` | Time to wait for a response before reporting a timeout error |
| Persistent connection | Checkbox | Off | When on, the connection stays open between poll cycles. Reconnects on error. TCP only |

## TCP settings

Visible when **Protocol** is `TCP`.

| Setting | Type | Example | Description |
| --- | --- | --- | --- |
| IP address | IPv4 string | `192.168.1.10` | Address of the Modbus TCP slave |
| Port | Integer 1–65535 | `502` | TCP port. The default Modbus port is `502` |

## RTU settings

Visible when **Protocol** is `RTU`.

| Setting | Type | Example | Description |
| --- | --- | --- | --- |
| Serial port | String | `COM3`, `/dev/ttyS0` | Local serial port name |
| Baud rate | Integer | `9600` | Serial speed in bits per second. Must match the device |
| Parity | `None` / `Even` / `Odd` | `None` | Must match the device |
| Data bits | `7` / `8` | `8` | Number of data bits per frame. Must match the device |
| Stop bits | `1` / `2` | `1` | Number of stop bits per frame. Must match the device |

## See also

- [Device settings](device-settings.md)
- [Configure a TCP connection (how-to)](../how-to/configure-tcp-connection.md)
- [Configure an RTU connection (how-to)](../how-to/configure-rtu-connection.md)
- [Explanation: Connections and devices](../explanation/connections-and-devices.md)
