#include "modbusregister.h"


ModbusRegister::ModbusRegister()
    : ModbusRegister(ModbusAddress(0), 0, ModbusDataType::Type::UNSIGNED_16)
{

}

ModbusRegister::ModbusRegister(ModbusAddress address, deviceId_t deviceId, ModbusDataType::Type type)
    : _address(address), _deviceId(deviceId), _type(type)
{

}

ModbusAddress ModbusRegister::address() const
{
    return _address;
}

void ModbusRegister::setAddress(ModbusAddress address)
{
    _address = address;
}

deviceId_t ModbusRegister::deviceId() const
{
    return _deviceId;
}

void ModbusRegister::setDeviceId(deviceId_t deviceId)
{
    _deviceId = deviceId;
}

void ModbusRegister::setType(ModbusDataType::Type type)
{
    _type = type;
}

ModbusDataType::Type ModbusRegister::type() const
{
    return _type;
}

QString ModbusRegister::description() const
{
    QString connStr = QString("dev %1").arg(deviceId() + 1);

    return QString("%1, %2, %3").arg(address().toString(), ModbusDataType::description(_type), connStr);
}

double ModbusRegister::processValue(uint16_t lowerRegister, uint16_t upperRegister, bool int32LittleEndian) const
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

ModbusRegister& ModbusRegister::operator= (const ModbusRegister& modbusRegister)
{
    // self-assignment guard
    if (this == &modbusRegister)
    {
        return *this;
    }

    _address = modbusRegister.address();
    _deviceId = modbusRegister.deviceId();
    _type = modbusRegister.type();

    // return the existing object so we can chain this operator
    return *this;
}

bool operator== (const ModbusRegister& reg1, const ModbusRegister& reg2)
{
    if ((reg1._address == reg2._address) && (reg1._deviceId == reg2._deviceId) && (reg1._type == reg2._type))
    {
        return true;
    }
    else
    {
        return false;
    }
}

QDebug operator<<(QDebug debug, const ModbusRegister &reg)
{
    QDebugStateSaver saver(debug);

    debug.nospace().noquote() << '['
                    << reg.description()
                    << ']';

    return debug;
}

QString ModbusRegister::dumpListToString(QList<ModbusRegister> list)
{
    QString str;
    QDebug dStream(&str);

    dStream << list;

    return str;
}

uint32_t ModbusRegister::convertEndianness(bool bLittleEndian, quint16 value, quint16 nextValue) const
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

double ModbusRegister::convertUint32ToFloat(quint32 value) const
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
    const double doubleValue = *(reinterpret_cast<float*>(&value));
#pragma GCC diagnostic pop

    switch(std::fpclassify(doubleValue))
    {
        case FP_INFINITE:  return 0.0f;
        case FP_NAN:       return 0.0f;
        case FP_NORMAL:    return doubleValue;
        case FP_SUBNORMAL: return doubleValue;
        case FP_ZERO:      return 0.0f;
        default:           return doubleValue;
    }
}

