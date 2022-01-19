#ifndef TESTSLAVEMODBUS_H
#define TESTSLAVEMODBUS_H

#include <QModbusTcpServer>
#include <QUrl>

#include "testslavedata.h"

class TestSlaveModbus : public QModbusTcpServer
{
    Q_OBJECT
public:
    explicit TestSlaveModbus(TestSlaveData *pTestSlaveData, QObject *parent = nullptr);
    ~TestSlaveModbus();

    bool connect(QUrl host, int slaveId);
    void disconnect();

    void setException(QModbusPdu::ExceptionCode exception, bool bPersistent);

signals:
    void requestProcessed();

protected:
    bool readData(QModbusDataUnit *newData) const;
    bool setMap(const QModbusDataUnitMap &map);
    bool writeData(const QModbusDataUnit &newData);

    QModbusResponse processRequest(const QModbusPdu &request);

private:

    bool verifyValidHoldingRegister(const QModbusDataUnit *dataUnit) const;

    TestSlaveData *_pTestSlaveData;

    QModbusPdu::ExceptionCode _exceptionCode;
    bool _bExceptionPersistent;

};

#endif // TESTSLAVEMODBUS_H
