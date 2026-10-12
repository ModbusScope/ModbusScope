# Device ID Sync Analysis

## Decision

A device ID is assigned once, when the device is created, and is **never editable** afterwards. This is a
permanent design decision, not a temporary workaround. Renaming a device or moving it to another adapter is
supported; changing its ID is not.

## Why the ID must not change

A device's ID lives in two places that must always agree:

| Layer           | Storage                               | ID location                         |
| --------------- | ------------------------------------- | ----------------------------------- |
| `SettingsModel` | `QMap<deviceId_t, Device> _devices`   | map key (`Device` has no ID member) |
| `AdapterData`   | adapter config JSON `"devices"` array | `"id"` field of each entry          |

The ID is also what data points and expressions use to address a device, so editing it would mean updating
references across the project as well as both layers. Not supporting edits removes that whole class of problem.

## How the two layers are kept consistent

### Single mutation point

`SettingsModel::applyDeviceList()` is the only way to change the device list in `src/`. It replaces the whole map
in one step and emits at most one `deviceListChanged()`. Every writer builds a complete map and applies it:
loading a project file, accepting the Settings dialog, and reconciling devices with adapters.

### Settings dialog (`AdapterDeviceSettings`)

- **Open:** calls `reconcileDevicesWithAdapters()`, then builds one `DeviceConfigTab` per device in the model's
  `deviceList()`. Devices whose owning adapter has not described are not shown as tabs; they are kept in
  `_deviceIdsWithoutTab` and handed back to the model unchanged when the dialog is accepted.
- **Add device:** `handleAddTab()` assigns one more than the highest ID held by the model or an open tab, so a new
  ID never collides with an existing one. Nothing reaches the model until accept.
- **Accept:** `acceptValues()` writes each adapter's `"devices"` array from the tabs, then rebuilds the device map
  from the same tabs and applies it with one `applyDeviceList()` call. Cancelling discards all edits.

### Read-only ID in `DeviceConfigTab`

The ID is protected at two levels:

1. **UI:** `rebuildSchemaForm()` sets `"readOnly": true` on the schema's `id` property; `SchemaFormWidget::addFieldRow()`
   disables the widget of a read-only property.
2. **Data:** `DeviceConfigTab::values()` always writes the tab's own `_deviceId` and ignores the widget's value.
   Even if the widget were edited, the JSON ID can't differ from the map key. Switching adapters keeps the ID
   (`onAdapterChanged()` reinserts it into the new adapter's defaults).

### Adapter reconciliation

`reconcileDevicesWithAdapters()` runs whenever an adapter describes, and again when the Settings dialog opens. For each
device ID an adapter's config declares, it makes sure the device exists in the model and assigns its owner. Adapters
with a stored config claim a shared ID before unconfigured ones, and `modbus` comes first among the unconfigured ones.
It never removes devices.

### Project file

- **Load:** `applyAdapterSettings()` stores each adapter's config; `applyDeviceSettings()` builds the device map from
  the file's `devices` section and applies it. A repeated device ID gets a warning, and later entries only fill in
  fields the earlier one left empty.
- **Save:** the `devices` section comes from `SettingsModel`, the `adapters` section from each adapter's stored config.
  Because both are written from the same tab ID on accept, they agree.
- **On the wire:** `AdapterData::configForWire()` drops repeated device IDs (first one wins) before capping to the
  adapter's device limit.

## Remaining loose coupling

The project file's `adapters` and `devices` sections are loaded independently and not cross-checked. A hand-edited
or legacy file can make them disagree:

- **Device only in an adapter config:** added to the model when that adapter describes (via reconciliation).
- **Device only in the `devices` section:** shown in the Settings dialog with the adapter's defaults, but missing
  from the adapter config, so it is not polled until the dialog is accepted.

This isn't caused by ID editing and doesn't need a fix for the read-only decision. If it ever causes problems,
the natural fix is to reconcile the two sections once, at the end of `ProjectFileHandler` loading.

## Tests

- `tst_deviceconfigtab`: `idFieldIsReadOnly`, `idFieldStaysReadOnlyAfterAdapterChange`, which check that the ID widget is disabled.
- `tst_adapterdevicesettings`: ID preservation across adapter switches and accept, unique IDs for added tabs,
  `invalidIdTabValuesOmitFabricatedId`.
- `tst_settingsmodel`: `applyDeviceList*` and reconciliation behaviour.

## Key files

| File                                      | Purpose                                                            |
| ----------------------------------------- | ------------------------------------------------------------------ |
| `src/models/device.h`                     | Device class (name, adapter ID; no ID member)                      |
| `src/models/settingsmodel.cpp`            | Device map, `applyDeviceList()`, `reconcileDevicesWithAdapters()`  |
| `src/models/adapterdata.cpp`              | Adapter config, `configForWire()`                                  |
| `src/dialogs/adapterdevicesettings.cpp`   | Devices page: building tabs, adding devices, `acceptValues()`      |
| `src/customwidgets/deviceconfigtab.cpp`   | Device tab: read-only ID, `values()`                               |
| `src/customwidgets/schemaformwidget.cpp`  | Disables `readOnly` schema fields                                  |
| `src/importexport/projectfilehandler.cpp` | Project file load and save                                         |
