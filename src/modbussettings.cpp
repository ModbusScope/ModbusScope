

#include "modbussettings.h"

ModbusSettings::ModbusSettings()
{
}

void ModbusSettings::copy(const ModbusSettings * pSettings)
{
    this->_ipAddress = QString(pSettings->_ipAddress);
    this->_port = pSettings->_port;
    this->_slaveId = pSettings->_slaveId;
    this->_pollTime = pSettings->_pollTime;
}


void ModbusSettings::setIpAddress(QString ip)
{
    this->_ipAddress = QString(ip);
}

QString ModbusSettings::getIpAddress()
{
    return this->_ipAddress;
}

void ModbusSettings::setPort(quint16 port)
{
    this->_port = port;
}

void ModbusSettings::setPollTime(quint32 pollTime)
{
    this->_pollTime = pollTime;
}

quint16 ModbusSettings::getPort()
{
    return this->_port;
}

void ModbusSettings::setSlaveId(quint8 id)
{
    this->_slaveId = id;
}

quint8 ModbusSettings::getSlaveId()
{
    return this->_slaveId;
}

quint32 ModbusSettings::getPollTime()
{
    return this->_pollTime;
}
