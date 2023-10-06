#ifndef TESTSLAVEMODBUS_H
#define TESTSLAVEMODBUS_H

#include <QModbusTcpServer>
#include <QUrl>

#include "testslavedata.h"

class TestSlaveModbus : public QModbusTcpServer
{
    Q_OBJECT
public:

    typedef QMap<QModbusDataUnit::RegisterType, TestSlaveData*> ModbusDataMap;

    explicit TestSlaveModbus(ModbusDataMap &testSlaveData, QObject *parent = nullptr);
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

    bool verifyValidObject(const QModbusDataUnit *dataUnit) const;

    QMap<QModbusDataUnit::RegisterType, TestSlaveData*> &_testSlaveData;

    QModbusPdu::ExceptionCode _exceptionCode;
    bool _bExceptionPersistent;

};

#endif // TESTSLAVEMODBUS_H
