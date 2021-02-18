# Diagnostics

## Diagnostic logging

During a log session, several errors can occur. ModbusScope has built-in logging to allow quick examination or resolution of the error. A slave can respond too slow, so that a timeout is generated. All register read are logged in the diagnostic logs together with the results. Communication or connection errors can be examined with these logs. Other common errors are Modbus exceptions, for example when a register configuration isn't correct. The response of the slave is logged together.

The log will also contains some basic information about *ModbusScope* and the system that is running *ModbusScope*. These logs will contain, for example, the operating system and specific version of the OS.

## Logs window

Opening the *diagnostic logs* windows can be done with *Communication > Diagnostic logs...*

![image](../_static/user_manual/diagnostic_logs.png)

All logs can be viewed in this window. The log list will be dynamically updated when no logs are made. By toggling the filters, you can hide/show specific categories of the logs.

Specific logs can be selected and copied to the clipboard by right-clicking on the selected logs. Another option would be to export all logs with the *Export Logs* button.

Extensive debug logging can be enabled to log extra (internal) information about the status of of every read. Enabling this option will increase the amount of logs that will be generated.