#ifndef MODBUSREGISTER_H
#define MODBUSREGISTER_H

#include <QObject>

class ModbusRegister
{
public:
    ModbusRegister();

    quint16 address() const;
    void setAddress(quint16 address);
    
    quint8 connectionId() const;
    void setConnectionId(quint8 connectionId);

    bool is32Bit() const;
    void set32Bit(bool b32Bit);

    bool isUnsigned() const;
    void setUnsigned(bool bUnsigned);

private:

    quint16 _address;
    quint8 _connectionId;
    bool _b32Bit;
    bool _bUnsigned;
};


#endif // MODBUSREGISTER_H
