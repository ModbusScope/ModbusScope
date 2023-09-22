#ifndef MODBUSADDRESS_H
#define MODBUSADDRESS_H

#include <QObject>

class ModbusAddress
{

public:

    enum class ObjectType
    {
        COIL = 0,
        DISCRETE_INPUT = 1,
        INPUT_REGISTER = 2,
        HOLDING_REGISTER = 3,
        UNKNOWN = 4
    };

    enum class Offset
    {
        WITH_OFFSET = 0,
        WITHOUT_OFFSET = 1,
    };

    ModbusAddress();
    ModbusAddress(quint16 address, ModbusAddress::ObjectType type);
    ModbusAddress(quint16 address);
    ModbusAddress(const ModbusAddress& t) = default;

    ~ModbusAddress() = default;

    ModbusAddress& operator=(const ModbusAddress& other) = default;

    ModbusAddress::ObjectType objectType() const;
    quint16 address(Offset offset) const;

    QString toString() const;

    ModbusAddress next() const;
    ModbusAddress next(int i) const;

    friend bool operator== (const ModbusAddress& reg1, const ModbusAddress& reg2);
    friend bool operator< (const ModbusAddress& reg1, const ModbusAddress& reg2);
    friend bool operator> (const ModbusAddress& reg1, const ModbusAddress& reg2);

private:
    ModbusAddress::ObjectType convertFromOffset(quint16 address);

    quint16 _address{0};
    ObjectType _type{ObjectType::UNKNOWN};
    
    static const quint16 cObjectTypeOffsets[];

};



#endif // MODBUSADDRESS_H
