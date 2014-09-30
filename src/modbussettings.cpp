

#include "modbussettings.h"

ModbusSettings::ModbusSettings()
{
}

void ModbusSettings::Copy(const ModbusSettings * pSettings)
{
    this->ipAddress = QString(pSettings->ipAddress);
    this->port = pSettings->port;
    this->slaveId = pSettings->slaveId;
}


void ModbusSettings::SetIpAddress(QString ip)
{
    this->ipAddress = QString(ip);
}

QString ModbusSettings::GetIpAddress()
{
    return this->ipAddress;
}

void ModbusSettings::SetPort(quint16 port)
{
    this->port = port;
}

quint16 ModbusSettings::GetPort()
{
    return this->port;
}

void ModbusSettings::SetSlaveId(quint8 id)
{
    this->slaveId = id;
}

quint8 ModbusSettings::GetSlaveId()
{
    return this->slaveId;
}
