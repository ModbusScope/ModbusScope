#ifndef DEVICE_H
#define DEVICE_H

#include <QString>

using deviceId_t = quint32;

class Device
{

public:
    explicit Device(deviceId_t devdId = cFirstDeviceId);
    ~Device() = default;

    void setName(QString const& name);
    void setAdapterId(const QString& adapterId);

    QString name();
    QString adapterId() const;

    static deviceId_t const cFirstDeviceId;

private:
    QString _name;
    QString _adapterId;
};

#endif // DEVICE_H
