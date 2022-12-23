#include "modbusregister.h"


ModbusRegister::ModbusRegister()
    : ModbusRegister(0, 0, ModbusDataType::Type::UNSIGNED_16)
{

}

ModbusRegister::ModbusRegister(quint32 address, quint8 connectionId, ModbusDataType::Type type)
    : _address(address), _connectionId(connectionId), _type(type)
{

}

quint32 ModbusRegister::address() const
{
    return _address;
}

void ModbusRegister::setAddress(quint32 address)
{
    _address = address;
}

quint8 ModbusRegister::connectionId() const
{
    return _connectionId;
}

void ModbusRegister::setConnectionId(quint8 connectionId)
{
    _connectionId = connectionId;
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
    QString connStr = QString("conn %1").arg(connectionId() + 1);

    if (ModbusDataType::isFloat(_type))
    {
        return QString("%1, float32, %2").arg(address())
                                        .arg(connStr);
    }
    else
    {
        QString unsignedStr = ModbusDataType::isUnsigned(_type) ? "unsigned" : "signed" ;
        QString typeStr = ModbusDataType::is32Bit(_type) ? "32 bit" : "16 bit" ;

        return QString("%1, %2, %3, %4").arg(address())
                                        .arg(unsignedStr, typeStr, connStr);
    }
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
    _connectionId = modbusRegister.connectionId();
    _type = modbusRegister.type();

    // return the existing object so we can chain this operator
    return *this;
}

bool operator== (const ModbusRegister& reg1, const ModbusRegister& reg2)
{
    if (
        (reg1._address == reg2._address)
        && (reg1._connectionId == reg2._connectionId)
        && (reg1._type == reg2._type)
    )
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
    const double doubleValue = *(reinterpret_cast<float*>(&value));

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

