#ifndef MODBUSMASTER_H
#define MODBUSMASTER_H

#include <modbus.h>
#include "string"
#include "QList"

class ModbusMaster
{
    public:
        ModbusMaster();
        ~ModbusMaster();

        int32_t Open(std::string ip, int32_t port);
        int32_t ReadRegisters(unsigned short startRegister, unsigned char num, QList<unsigned short> * pList);
        void Close();

    private:

    modbus_t * ctx;

    uint16_t tab_reg[64];

};

#endif // ModbusMaster_H
