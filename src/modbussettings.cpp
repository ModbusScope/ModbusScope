

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
    this->_timeout = pSettings->_timeout;
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

quint16 ModbusSettings::getPort()
{
    return this->_port;
}

void ModbusSettings::setPollTime(quint32 pollTime)
{
    this->_pollTime = pollTime;
}

quint32 ModbusSettings::getPollTime()
{
    return this->_pollTime;
}

quint8 ModbusSettings::getSlaveId()
{
    return this->_slaveId;
}

void ModbusSettings::setSlaveId(quint8 id)
{
    this->_slaveId = id;
}

quint32 ModbusSettings::getTimeout()
{
    return this->_timeout;
}

void ModbusSettings::setTimeout(quint32 timeout)
{
    this->_timeout = timeout;
}
