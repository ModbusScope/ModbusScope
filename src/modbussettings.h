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

    QString getIpAddress();
    quint16 getPort();
    quint8 getSlaveId();

private:
    QString _ipAddress;
    quint16 _port;
    quint8 _slaveId;

};

#endif // MODBUSSETTINGS_H
