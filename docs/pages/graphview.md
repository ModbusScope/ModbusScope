# Graph view

## Start/stop log

Once some registers are added, the actual logging can be started. *ModbusScope* will communicate with the Modbus TCP slave defined in the connection settings. This slave can also be a Modbus TCP to Modbus RTU bridge like *ModbusControl*. Make sure *ModbusControl* is active and communication to the Modbus RTU slave is working correctly.

Then press the *Start Logging* button. *ModbusScope* will start logging and will automatically add the values to the graph.

> **NOTE**: When restarting the logging using the *Start Logging* button any data already present in the graph will be cleared.

![image](../_static/user_manual/start_logging.png)

Once your test is finished the logging can be stopped by pressing the *Stop Logging* button. The logging will stop and the values can be inspected further.

![image](../_static/user_manual/stop_logging.png)

## Adjust scale settings

While ModbusScope is logging, the already logged values can be inspected in the graph view. With the different scale settings, the data can be examined while new values are still be being added to the log. Several scale settings are available.

### X-axis

There are 3 options for the scaling of the x-axis: *full auto-scale*, *sliding window* and *manual* settings. When *full auto-scale* is selected, the x-axis will automatically scale the maximum of the x-axis to keep all time values in the graph. In the *sliding window* setting only the values of a specific last time span will be visible. This time span is configurable. *Manual* scale setting means that the scaling isn't changed automatically while logging. Even when new values are logged, the current time period stays the same.

### Y1- and Y2-axis

Compared to the x-axis setting, the y-axis shares 2 settings: *full auto-scale* and *manual*. These settings are the same as with the x-axis. The 2 other modes are *window auto-scale* and the *limit from* setting. The *window auto-scale* automatically scales the y-axis based on the values that are currently visible. The *limit from* setting allows the user to configure the minimum and maximum values of the axis. There are 2 Y-axis available. An Y-axis can be selected per graph.

## Zoom graph

The graph view supports zooming to examine the log in detail. When the scroll wheel of the mouse is used, both axis switch to manual setting and the range of the axis is increased or decreased based on the scroll wheel. The current position of the mouse cursor is used as reference point for the zoom action.

It is possible to select a single axis to only zoom this axis by clicking it. The range of the other axis will remain the same when zooming (mouse wheel scroll). An axis can be unselected by clicking somewhere in the graph view. An axis can be reset to full auto-scale setting by double clicking it.

## Enable/Disable markers

*ModbusScope* can help in investigating dynamic behavior of a system by measuring times and value differences between two points in the graph. First two vertical markers have to be added.

Add the left marker (green vertical line) by pressing *\<Ctrl>* and *\<Left mouse click>* on the place of interest. The position must coincide with a sample in the graph. Add the right marker (red vertical line) by pressing *\<Ctrl>* and *\<Right mouse click>* on the place of interest.

On the right hand side the *Markers* panel will show information about the markers. Such as the value of the registers at *Time 1* (left hand marker) and *Time 2* (right hand marker). Also the time difference between the two markers or value difference between the two marked times.

![image](../_static/user_manual/markers.png)
