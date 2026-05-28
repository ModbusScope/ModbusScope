# Device settings reference

Open via **Settings > Device**.

A device represents one Modbus slave. Each device is linked to a connection and carries the Modbus-protocol parameters for that slave.

## Settings

| Setting | Type | Default | Description |
| --- | --- | --- | --- |
| Name | String | — | User-visible label for the device |
| ID | Integer | — | Internal ID used in register expressions (`@DEVICE`) |
| Connection ID | Integer | `1` | Which connection this device communicates over |
| Slave ID | Integer 1–247 | `1` | Modbus unit address of the slave |
| Timeout | Integer (ms) | `1000` | Time to wait for a response from this device before reporting a timeout. Overrides the connection-level timeout for this device |
| Max consecutive registers | Integer | `125` | Maximum number of registers fetched in a single Modbus read request. Reduce if the device rejects large requests |
| 32-bit little-endian | Checkbox | Off | When on, 32-bit values (`32b`, `s32b`, `f32b`) are read in little-endian word order. Must match the device |

## See also

- [Connection settings](connection-settings.md)
- [Register syntax — @DEVICE](register-syntax.md)
- [Explanation: Connections and devices](../explanation/connections-and-devices.md)
- [Optimize poll rate (how-to)](../how-to/optimize-poll-rate.md)
