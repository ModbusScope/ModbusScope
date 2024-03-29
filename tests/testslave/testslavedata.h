#ifndef TESTSLAVEDATA_H
#define TESTSLAVEDATA_H

#include <QObject>
#include <QMap>
#include <QModbusServer>

class TestSlaveData : public QObject
{
    Q_OBJECT
public:
    explicit TestSlaveData(quint32 offset = 0, quint32 registerCount = 50);
    ~TestSlaveData();

    bool IsValidAddress(quint32 startAddress, quint32 valueCount);

    void setRegisterState(uint registerAddress, bool bState);
    void setRegisterState(QList<uint> registerAddressList, bool bState);

    void setRegisterValue(uint registerAddress, quint16 value);

    bool registerState(uint registerAddress);
    quint16 registerValue(uint registerAddress);

    void incrementAllEnabledRegisters();

signals:
    void dataChanged();

private:

    typedef struct
    {
        bool bState;
        quint16 value;
    } registerData_t;

    QList<registerData_t> _registerList;

    quint32 _offset;

};

#endif // TESTSLAVEDATA_H
