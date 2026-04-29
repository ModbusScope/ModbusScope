# Device ID Sync Analysis

## Issue Summary

When a user changes the device ID in `DeviceConfigTab`, the change is saved to the adapter's JSON config but **does NOT propagate to SettingsModel's device map**. This causes:
- ID change lost on next file save (file uses SettingsModel)
- Potential ID collision if user later adds devices
- Inconsistent state between two sources of truth

## Root Cause

Dual-layer architecture with loose coupling:

| Layer | Storage | ID Access |
|-------|--------|----------|
| SettingsModel | `_devices` map | ID = map key |
| AdapterData | JSON `"devices"` array | `"id"` field |

The `Device` class (`device.h:23-26`) only stores `_name` and `_adapterId` — not its own ID.

**Data flow gap**: When ID is edited in UI → SchemaForm updates JSON → `acceptValues()` writes JSON to adapter config → SettingsModel map key unchanged.

### Code Locations

- **SettingsModel device map**: `settingsmodel.cpp:144-150` — `_devices[devId]` returns Device* where ID is the map key
- **JSON config storage**: Adapter's `currentConfig["devices"][N]["id"]` — stored in adapter data
- **SettingsModel::updateDeviceId()**: `settingsmodel.cpp:229-237` — exists but is never called from UI

### Synchronization Flow

On **load** (`projectfilehandler.cpp:277-303`):
1. Creates Devices in `_devices` map from file's deviceId
2. Sets adapterId on each Device
3. **Never syncs back to adapter config**

On **dialog init** (`adapterdevicesettings.cpp:35-82`):
1. Reads IDs from JSON `deviceObj.value("id")`
2. Calls `_pSettingsModel->addDevice(id)`
3. Creates DeviceConfigTab widgets
4. **Does NOT populate adapter config from SettingsModel**

On **save** (`projectfilehandler.cpp:185-212`):
1. Reads from **SettingsModel** (`_pSettingsModel->deviceList()`)
2. Uses model name/adapterId
3. **Ignores ID changes in JSON**

On **accept dialog** (`adapterdevicesettings.cpp:185-208`):
1. Reads from DeviceConfigTab::values() → SchemaForm::values() (JSON)
2. Writes JSON to adapter config via `setAdapterCurrentConfig()`
3. **SettingsModel map key unchanged**

### Key Files

| File | Purpose |
|------|---------|
| `src/models/device.h` | Device class (no ID member) |
| `src/models/settingsmodel.cpp` | Device map + updateDeviceId() |
| `src/dialogs/adapterdevicesettings.cpp` | Dialog acceptValues() |
| `src/customwidgets/deviceconfigtab.cpp` | Device config tab |
| `src/customwidgets/schemaformwidget.cpp` | Form field rendering |
| `src/importexport/projectfilehandler.cpp` | File load/save |

## Solution Applied (Temporary)

Made the device ID field **read-only** in the UI. The ID is assigned at device creation and should not be changed afterward.

### Changes Made

1. **schemaformwidget.cpp**: Added readOnly handling in `createWidgetForProperty()`
   - Check schema property for `"readOnly": true` 
   - Disable widgets when property is read-only

2. **deviceconfigtab.cpp**: Inject readOnly into `id` property in `rebuildSchemaForm()`
   - Before passing schema to SchemaFormWidget, inject `"readOnly": true` into the `id` property

### Future Fix (Proper Sync)

Option 1: Call `SettingsModel::updateDeviceId(oldId, newId)` when ID edited
- Requires detecting ID change in acceptValues()
- Map key needs to be renamed in _devices map

Option 2: Refactor to single source of truth
- Add `_id` member to `Device` class
- Remove ID from map key, use QMap<quint32, Device> instead
- Sync ID changes bidirectionally

Option 3: Event-driven sync
- Emit signal when ID changes in either layer
- Listen and propagate to other layer

## Risk Assessment

- **Current**: Changing ID will cause data loss/inconsistency
- **After readOnly fix**: ID cannot be changed, data stays consistent
- **Adding/deleting devices**: Works correctly
- **Just saving without editing ID**: Works correctly