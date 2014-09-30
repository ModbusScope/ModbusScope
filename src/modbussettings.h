#ifndef MODBUSSETTINGS_H
#define MODBUSSETTINGS_H

#include "QString"
#include "QList"

class ModbusSettings {

public:
    ModbusSettings();

    void Copy(const ModbusSettings * pSettings);

    void SetIpAddress(QString ip);
    void SetPort(quint16 port);
    void SetSlaveId(quint8 id);

    QString GetIpAddress();
    quint16 GetPort();
    quint8 GetSlaveId();

private:
    QString ipAddress;
    quint16 port;
    quint8 slaveId;

};

#endif // MODBUSSETTINGS_H
