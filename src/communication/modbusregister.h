#ifndef MODBUSREGISTER_H
#define MODBUSREGISTER_H

#include <QObject>

class ModbusRegister
{
public:
    ModbusRegister();
    ModbusRegister(quint16 address, quint8 connectionId, bool is32Bit, bool isUnsigned);

    quint16 address() const;
    void setAddress(quint16 address);
    
    quint8 connectionId() const;
    void setConnectionId(quint8 connectionId);

    bool is32Bit() const;
    void set32Bit(bool b32Bit);

    bool isUnsigned() const;
    void setUnsigned(bool bUnsigned);

    // Copy constructor
    ModbusRegister(const ModbusRegister& copy)
        : _address { copy.address() }, _connectionId { copy.connectionId() },
          _b32Bit { copy.is32Bit() }, _bUnsigned { copy.isUnsigned() }
    {

    }

    ModbusRegister& operator= (const ModbusRegister& modbusRegister);

    friend bool operator== (const ModbusRegister& reg1, const ModbusRegister& reg2);

private:

    quint16 _address;
    quint8 _connectionId;
    bool _b32Bit;
    bool _bUnsigned;
};

#endif // MODBUSREGISTER_H
