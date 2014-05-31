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

    QString GetIpAddress();
    quint16 GetPort();

private:
    QString ipAddress;
    quint16 port;

};

#endif // MODBUSSETTINGS_H
