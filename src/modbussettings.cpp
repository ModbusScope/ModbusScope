

#include "modbussettings.h"

ModbusSettings::ModbusSettings()
{
}

void ModbusSettings::Copy(const ModbusSettings * pSettings)
{
    this->ipAddress = QString(pSettings->ipAddress);
    this->port = pSettings->port;
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
