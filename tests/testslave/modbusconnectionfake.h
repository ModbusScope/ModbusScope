#ifndef MODBUSCONNECTIONFAKE_H
#define MODBUSCONNECTIONFAKE_H

#include "communication/modbusconnection.h"
#include "communication/modbusdataunit.h"
#include "testdevice.h"

using SlaveDeviceMap = QMap<ModbusAddress::slaveId_t, TestDevice*>;

class ModbusConnectionFake : public ModbusConnection
{
    Q_OBJECT
public:
    explicit ModbusConnectionFake();

    void addSlaveDevice(ModbusAddress::slaveId_t slaveId, TestDevice* pDevice);

    void configureTcpConnection(tcpSettings_t const& tcpSettings) override;
    void configureSerialConnection(serialSettings_t const& serialSettings) override;

    void open(quint32 timeout) override;
    void close(void) override;

    void sendReadRequest(ModbusDataUnit const& regAddress, quint16 size) override;

    bool isConnected(void) override;

private:
    SlaveDeviceMap _deviceMap;
};

#endif // MODBUSCONNECTIONFAKE_H
