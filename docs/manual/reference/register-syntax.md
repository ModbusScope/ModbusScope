# Register syntax reference

## Expression form

```text
${REG[@DEVICE][:TYPE]}
```

| Part | Required | Description |
| --- | --- | --- |
| `REG` | Yes | Register address in Modicon or prefix notation (see below) |
| `@DEVICE` | No | Device ID to read from. Defaults to device 1 if omitted |
| `:TYPE` | No | Data type. Defaults to `16b` if omitted. Ignored for coils and discrete inputs |

### Examples

| Expression | Meaning |
| --- | --- |
| `${40001}` | Holding register 40001, unsigned 16-bit, device 1 |
| `${40001: s16b}` | Holding register 40001, signed 16-bit, device 1 |
| `${40001@2: 32b}` | Holding register 40001, unsigned 32-bit, device 2 |
| `${i0}` | Input register at address 0, unsigned 16-bit, device 1 |
| `${c5@3}` | Coil 5, device 3 |

---

## Address notation

Two notations are supported and can be mixed in the same session.

### Modicon 5-digit notation

Encodes the object type and address in one number. Limited to addresses 1–9999 within each type.

| Range | Object type | Modbus function code |
| --- | --- | --- |
| `1` – `9999` | Coil | 1 |
| `10001` – `19999` | Discrete input | 2 |
| `30001` – `39999` | Input register | 4 |
| `40001` and above | Holding register | 3 |

Address `40001` in Modicon notation maps to Modbus address `0` on the wire.

### Prefix notation

Use a letter prefix followed by the full 0–65535 address. Required when the address exceeds 9999 within its type.

| Prefix | Object type | Modbus function code |
| --- | --- | --- |
| `c0` – `c65535` | Coil | 1 |
| `d0` – `d65535` | Discrete input | 2 |
| `i0` – `i65535` | Input register | 4 |
| `h0` – `h65535` | Holding register | 3 |

ModbusScope stores addresses in Modicon notation when they fit; otherwise it uses prefix notation.

---

## Data types

Applies to holding registers and input registers only.

| Type | Size | Signed | Notes |
| --- | --- | --- | --- |
| `16b` | 16-bit | No | Default |
| `s16b` | 16-bit | Yes | |
| `32b` | 32-bit | No | Two consecutive registers |
| `s32b` | 32-bit | Yes | Two consecutive registers |
| `f32b` | 32-bit | IEEE 754 float | Two consecutive registers |

The byte order of 32-bit types is controlled by the **32-bit little-endian** setting on the device. See [Device settings](device-settings.md).

For coils and discrete inputs, `:TYPE` is accepted but ignored. The result is always `0` or `1`.
