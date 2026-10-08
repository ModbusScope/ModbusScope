# Log settings reference

Open via **Project > Settings...** and select the **Log** page.

## Settings

| Setting | Type | Default | Description |
| --- | --- | --- | --- |
| Poll time (ms) | Integer (ms) | `250` | Target time between poll cycles. The actual interval may be longer depending on Modbus response time |
| Time reference | `Relative` / `Absolute` | `Relative` | **Relative (use start of log as time reference)** logs elapsed time since the start of the session. **Absolute (use current time/date as time reference)** logs absolute date-time values |
| Save data to temporary file while logging | Checkbox | On | When on, data is written to a temporary file as it is logged. Allows recovery if the application crashes before export |
| Temporary file location | Path string | System temp folder | Location of the temporary data file. Cleared at the start of each new logging session |

## See also

- [Optimize poll rate (how-to)](../how-to/optimize-poll-rate.md)
- [Explanation: Polling and sample rate](../explanation/polling-and-sample-rate.md)
