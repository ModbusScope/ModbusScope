#ifndef DATAPOINT_H
#define DATAPOINT_H

#include "models/device.h"
#include <QDebug>
#include <QObject>
#include <QString>

class DataPoint
{
public:
    DataPoint();
    DataPoint(QString const& address, deviceId_t deviceId);

    QString address() const;

    deviceId_t deviceId() const;

    QString description() const;

    DataPoint(const DataPoint& copy) : _address{ copy.address() }, _deviceId{ copy.deviceId() }
    {
    }

    DataPoint& operator=(const DataPoint& dataPoint);

    friend bool operator==(const DataPoint& dp1, const DataPoint& dp2);

    static QString dumpListToString(QList<DataPoint> list);

private:
    QString _address;
    deviceId_t _deviceId;
};

QDebug operator<<(QDebug debug, const DataPoint& dp);

#endif // DATAPOINT_H
