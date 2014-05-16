#ifndef MODBUSSETTINGS_H
#define MODBUSSETTINGS_H

#include "QString"
#include "QList"

class ModbusSettings {

public:
    ModbusSettings();

    void Copy(const ModbusSettings * pSettings);

    void SetIpAddress(QString ip);
    void SetPort(int32_t port);
    void SetRegisters(QList<u_int16_t> * pRegisters);



    QString GetIpAddress();
    int32_t GetPort();
    void GetRegisters(QList<u_int16_t> * pRegisters);
    u_int32_t GetRegisterCount();

private:
    QString ipAddress;
    int32_t port;
    QList<u_int16_t> regList;
};

#endif // MODBUSSETTINGS_H
