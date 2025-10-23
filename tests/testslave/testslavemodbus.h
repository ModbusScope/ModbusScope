#ifndef TESTSLAVEMODBUS_H
#define TESTSLAVEMODBUS_H

#include <QModbusTcpServer>

#include "testdevice.h"

class TestSlaveModbus : public QModbusTcpServer
{
    Q_OBJECT
public:
    explicit TestSlaveModbus(QObject* parent = nullptr);
    ~TestSlaveModbus();

    bool connect(QString ip, quint16 port, int slaveId);
    void disconnect();

    void setException(QModbusPdu::ExceptionCode exception, bool bPersistent);

    TestDevice* testDevice()
    {
        return &_testDevice;
    }

signals:
    void requestProcessed();

protected:
    bool readData(QModbusDataUnit* newData) const;
    bool setMap(const QModbusDataUnitMap& map);
    bool writeData(const QModbusDataUnit& newData);

    QModbusResponse processRequest(const QModbusPdu& request);

private:
    bool verifyValidObject(const QModbusDataUnit* dataUnit) const;

    TestDevice _testDevice;

    QModbusPdu::ExceptionCode _exceptionCode;
    bool _bExceptionPersistent;
};

#endif // TESTSLAVEMODBUS_H
