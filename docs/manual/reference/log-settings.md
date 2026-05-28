# Log settings reference

Open via **Settings > Log**.

## Settings

| Setting | Type | Default | Description |
| --- | --- | --- | --- |
| Poll interval | Integer (ms) | `250` | Target time between poll cycles. The actual interval may be longer depending on Modbus response time |
| Use absolute timestamps | Checkbox | Off | When off, timestamps are relative to session start (milliseconds elapsed). When on, absolute date-time values are logged |
| Write temporary file | Checkbox | On | When on, data is written to a temporary file as it is logged. Allows recovery if the application crashes before export |
| Temporary file path | Path string | System temp folder | Location of the temporary data file. Cleared at the start of each new logging session |

## See also

- [Optimize poll rate (how-to)](../how-to/optimize-poll-rate.md)
- [Explanation: Polling and sample rate](../explanation/polling-and-sample-rate.md)
