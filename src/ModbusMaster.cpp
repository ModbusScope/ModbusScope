

#include <errno.h>
#include <iostream>


#include "ModbusMaster.h"
#include "QList"

using namespace std;


ModbusMaster::ModbusMaster()
{
    ctx = NULL;
}


ModbusMaster::~ModbusMaster()
{

}


int32_t ModbusMaster::Open(string ip, int32_t port)
{
    int32_t ret = 0;

    ctx = modbus_new_tcp(ip.c_str(), port);
    if (modbus_connect(ctx) == -1)
    {
        cerr << "Connection failed: " << modbus_strerror(errno) << endl;
        modbus_free(ctx);
        ret = -1;
    }

    tab_reg[0] = 0;
    tab_reg[1] = 0;

    return ret;

}

int32_t ModbusMaster::ReadRegisters(unsigned short startRegister, unsigned char num, QList<unsigned short> * pList)
{
    int32_t rc = -1;

    rc = modbus_read_registers(ctx, startRegister, num, tab_reg);
    if (rc == -1)
    {
        cerr << "Connection failed: " << modbus_strerror(errno) << endl;
    }
    else
    {

        pList->clear();
        for (u_int32_t i = 0; i < num; i++)
        {
            pList->append(tab_reg[i]);
        }
    }

    return rc;
}


void ModbusMaster::Close()
{
    modbus_close(ctx);
    modbus_free(ctx);
}


