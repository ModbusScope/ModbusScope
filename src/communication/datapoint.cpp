#include "datapoint.h"

#include <bit>

DataPoint::DataPoint() : DataPoint(QString(), Device::cFirstDeviceId, ModbusDataType::Type::UNSIGNED_16)
{
}

DataPoint::DataPoint(QString const& address, deviceId_t deviceId, ModbusDataType::Type type)
    : _address(address), _deviceId(deviceId), _type(type)
{
}

QString DataPoint::address() const
{
    return _address;
}

void DataPoint::setAddress(const QString& address)
{
    _address = address;
}

deviceId_t DataPoint::deviceId() const
{
    return _deviceId;
}

void DataPoint::setDeviceId(deviceId_t deviceId)
{
    _deviceId = deviceId;
}

void DataPoint::setType(ModbusDataType::Type type)
{
    _type = type;
}

ModbusDataType::Type DataPoint::type() const
{
    return _type;
}

QString DataPoint::description() const
{
    QString connStr = QString("device id %1").arg(deviceId());

    return QString("%1, %2, %3").arg(_address, ModbusDataType::description(_type), connStr);
}

double DataPoint::processValue(uint16_t lowerRegister, uint16_t upperRegister, bool int32LittleEndian) const
{
    double processedResult = 0u;

    if (ModbusDataType::isFloat(_type))
    {
        uint32_t combinedValue = convertEndianness(int32LittleEndian, lowerRegister, upperRegister);

        processedResult = convertUint32ToFloat(combinedValue);
    }
    else if (ModbusDataType::is32Bit(_type))
    {
        uint32_t combinedValue = convertEndianness(int32LittleEndian, lowerRegister, upperRegister);

        if (ModbusDataType::isUnsigned(_type))
        {
            processedResult = static_cast<double>(static_cast<quint32>(combinedValue));
        }
        else
        {
            processedResult = static_cast<double>(static_cast<qint32>(combinedValue));
        }
    }
    else
    {
        if (ModbusDataType::isUnsigned(_type))
        {
            processedResult = static_cast<double>(static_cast<quint16>(lowerRegister));
        }
        else
        {
            processedResult = static_cast<double>(static_cast<qint16>(lowerRegister));
        }
    }

    return processedResult;
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
    _type = dataPoint.type();

    // return the existing object so we can chain this operator
    return *this;
}

bool operator==(const DataPoint& dp1, const DataPoint& dp2)
{
    if ((dp1._address == dp2._address) && (dp1._deviceId == dp2._deviceId) && (dp1._type == dp2._type))
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

QString DataPoint::dumpListToString(QList<DataPoint> list)
{
    QString str;
    QDebug dStream(&str);

    dStream << list;

    return str;
}

uint32_t DataPoint::convertEndianness(bool bLittleEndian, quint16 value, quint16 nextValue) const
{
    uint32_t combinedValue;
    if (bLittleEndian)
    {
        combinedValue = (static_cast<uint32_t>(nextValue) << 16) | value;
    }
    else
    {
        combinedValue = (static_cast<uint32_t>(value) << 16) | nextValue;
    }

    return combinedValue;
}

double DataPoint::convertUint32ToFloat(quint32 value) const
{
    const double doubleValue = std::bit_cast<float>(value);

    switch (std::fpclassify(doubleValue))
    {
    case FP_INFINITE:
    case FP_NAN:
    case FP_ZERO:
        return 0.0f;
    case FP_NORMAL:
    case FP_SUBNORMAL:
    default:
        return doubleValue;
    }
}
