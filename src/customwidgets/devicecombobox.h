#ifndef DEVICECOMBOBOX_H
#define DEVICECOMBOBOX_H

#include "models/device.h"

#include <QComboBox>

class SettingsModel;

class DeviceComboBox : public QComboBox
{
    Q_OBJECT

public:
    enum class LabelStyle
    {
        Name,
        NameProtocolId
    };

    explicit DeviceComboBox(QWidget* parent = nullptr);

    void setSettingsModel(SettingsModel* pSettingsModel);
    void setLabelStyle(LabelStyle style);

    deviceId_t selectedDeviceId() const;
    void setSelectedDeviceId(deviceId_t deviceId);

signals:
    void deviceChanged(deviceId_t deviceId);

private slots:
    void onCurrentIndexChanged(int index);
    void onDeviceListChanged();

private:
    void populate();
    QString deviceLabel(deviceId_t deviceId) const;

    SettingsModel* _pSettingsModel{ nullptr };
    LabelStyle _labelStyle{ LabelStyle::Name };
};

#endif // DEVICECOMBOBOX_H
