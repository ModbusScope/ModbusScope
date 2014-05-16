

#include "modbussettings.h"

ModbusSettings::ModbusSettings()
{
}

void ModbusSettings::Copy(const ModbusSettings * pSettings)
{
    this->ipAddress = QString(pSettings->ipAddress);
    this->port = pSettings->port;
    this->regList.clear();
    this->regList.append(pSettings->regList);
}


void ModbusSettings::SetIpAddress(QString ip)
{
    this->ipAddress = QString(ip);
}

void ModbusSettings::SetRegisters(QList<u_int16_t> * pRegisters)
{
    regList.clear();
    regList.append(*pRegisters);
}

QString ModbusSettings::GetIpAddress()
{
    return this->ipAddress;
}

void ModbusSettings::SetPort(int32_t port)
{
    this->port = port;
}

int32_t ModbusSettings::GetPort()
{
    return this->port;
}


void ModbusSettings::GetRegisters(QList<u_int16_t> * pRegisters)
{
    pRegisters->clear();
    pRegisters->append(regList);
}

u_int32_t ModbusSettings::GetRegisterCount()
{
    return regList.size();
}
