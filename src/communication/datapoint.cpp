#include "datapoint.h"

DataPoint::DataPoint() : DataPoint(QString(), Device::cFirstDeviceId)
{
}

DataPoint::DataPoint(QString const& address, deviceId_t deviceId) : _address(address), _deviceId(deviceId)
{
}

QString DataPoint::address() const
{
    return _address;
}

deviceId_t DataPoint::deviceId() const
{
    return _deviceId;
}

QString DataPoint::description() const
{
    QString connStr = QString("device id %1").arg(deviceId());

    return QString("%1, %2").arg(_address, connStr);
}

DataPoint& DataPoint::operator=(const DataPoint& dataPoint)
{
    // self-assignment guard
    if (this == &dataPoint)
    {
        return *this;
    }

    _address = dataPoint.address();
    _deviceId = dataPoint.deviceId();

    // return the existing object so we can chain this operator
    return *this;
}

bool operator==(const DataPoint& dp1, const DataPoint& dp2)
{
    if ((dp1._address == dp2._address) && (dp1._deviceId == dp2._deviceId))
    {
        return true;
    }
    else
    {
        return false;
    }
}

QDebug operator<<(QDebug debug, const DataPoint& dp)
{
    QDebugStateSaver saver(debug);

    debug.nospace().noquote() << '[' << dp.description() << ']';

    return debug;
}

QString DataPoint::dumpListToString(QList<DataPoint> const& list)
{
    QString str;
    QDebug dStream(&str);

    dStream << list;

    return str;
}