#ifndef TEST_DEVICE_H
#define TEST_DEVICE_H

#include "testslavedata.h"
#include <QUrl>
#include <qmodbusdataunit.h>

class TestDevice : public QObject
{
    Q_OBJECT
public:
    explicit TestDevice(QObject* parent = nullptr);
    ~TestDevice();

    TestSlaveData* slaveData(QModbusDataUnit::RegisterType type) const;
    void setSlaveData(QModbusDataUnit::RegisterType type, TestSlaveData* slaveData);

    void configureHoldingRegister(uint address, bool state, quint16 value);
    void configureInputRegister(uint address, bool state, quint16 value);
    void configureCoil(uint address, bool state, bool value);
    void configureDiscreteInput(uint address, bool state, bool value);

private:
    void configureRegister(QModbusDataUnit::RegisterType type, uint address, bool state, quint16 value);

    typedef QMap<QModbusDataUnit::RegisterType, TestSlaveData*> ModbusDataMap;

    ModbusDataMap _slaveDataMap;
};

#endif // TEST_DEVICE_H
