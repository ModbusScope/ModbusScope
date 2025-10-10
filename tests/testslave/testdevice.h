#ifndef TEST_DEVICE_H
#define TEST_DEVICE_H

#include <QUrl>

#include "testslavemodbus.h"

class TestDevice : public QObject
{
    Q_OBJECT
public:
    explicit TestDevice(QObject* parent = nullptr);
    ~TestDevice();

    bool connect(QString ip, quint16 port, int slaveId);
    void disconnect();

    void setException(QModbusPdu::ExceptionCode exception, bool bPersistent);

    void configureHoldingRegister(uint address, bool state, quint16 value);
    void configureInputRegister(uint address, bool state, quint16 value);
    void configureCoil(uint address, bool state, bool value);
    void configureDiscreteInput(uint address, bool state, bool value);

private:
    void configureRegister(QModbusDataUnit::RegisterType type, uint address, bool state, quint16 value);

    QUrl _connectionData;
    TestSlaveModbus::ModbusDataMap _slaveDataMap;
    TestSlaveModbus* _pSlaveModbus;
};

#endif // TEST_DEVICE_H
