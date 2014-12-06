#ifndef MODBUSSETTINGS_H
#define MODBUSSETTINGS_H

#include "QString"
#include "QList"

class ModbusSettings {

public:
    ModbusSettings();

    void copy(const ModbusSettings * pSettings);

    void setIpAddress(QString ip);
    void setPort(quint16 port);
    void setSlaveId(quint8 id);
    void setPollTime(quint32 pollTime);
    void setTimeout(quint32 timeout);

    QString getIpAddress();
    quint16 getPort();
    quint8 getSlaveId();
    quint32 getPollTime();
    quint32 getTimeout();

private:
    QString _ipAddress;
    quint16 _port;
    quint8 _slaveId;
    quint32 _pollTime;
    quint32 _timeout;

};

#endif // MODBUSSETTINGS_H
