#ifndef TST_DEVICECOMBOBOX_H
#define TST_DEVICECOMBOBOX_H

#include "models/settingsmodel.h"

#include <QObject>

class DeviceComboBox;

class TestDeviceComboBox : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void fillsFromModel();
    void nameProtocolIdLabels();
    void nameWithPlaceMarkerKeptInLabel();
    void setSelectedDeviceEmitsDeviceChanged();
    void selectionKeptWhenOtherDeviceAdded();
    void selectionKeptWhenOtherDeviceRemoved();
    void selectedDeviceRemovedFallsBackToFirst();
    void lastDeviceRemovedEmitsZero();

private:
    SettingsModel* _pSettingsModel{ nullptr };
    DeviceComboBox* _pCombo{ nullptr };
};

#endif // TST_DEVICECOMBOBOX_H
