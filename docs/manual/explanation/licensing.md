# Licensing

Some adapters, including the bundled Modbus adapter, enforce limits (for example on the number of devices or data points) unless a license is installed. This page explains where to see an adapter's license status, how to request and install a license, and what happens when a limit is exceeded.

## Where you see license status

Go to **Help > About...**. Each configured adapter is listed there along with its current license state.

## Getting a license

Click **Request License** in the About dialog to open [modbusscope.com](https://modbusscope.com/), where current licensing terms and how to request one are described.

## Installing a license

Click **Load License...** in the About dialog and select the license file for the adapter. The adapter verifies the file first, and ModbusScope shows who the license was issued to (customer, email, license ID and expiry date). The license is installed only after you confirm. Only load a license that was issued to you or your organisation. If a valid license is already installed, the confirmation tells you it will be replaced.

A license that is expired, tampered with or otherwise invalid is never installed; ModbusScope shows the reason instead. The adapter must be running to verify a license, and older adapters that cannot verify licenses need to be updated first.

The license takes effect the next time the adapter is initialized, for example after restarting ModbusScope.

## Limitations without a license

Limits vary by adapter and may change over time, so they are not listed here. See [modbusscope.com](https://modbusscope.com/) for current details. If your configuration exceeds what an adapter currently allows, ModbusScope shows a warning in the device settings and signals dialogs that points back to **Help > About > Request License**.

## See also

- [Reference: Device settings](../reference/device-settings.md)
- [Add signals](../how-to/add-data-points.md)
