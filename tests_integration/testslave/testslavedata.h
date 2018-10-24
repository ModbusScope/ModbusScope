#ifndef TESTSLAVEDATA_H
#define TESTSLAVEDATA_H

#include <QObject>
#include <QModbusServer>

class TestSlaveData : public QObject
{
    Q_OBJECT
public:
    explicit TestSlaveData(QModbusServer *pModbusServer);
    ~TestSlaveData();

signals:

public slots:

private slots:
    void onDataChanged(QModbusDataUnit::RegisterType reg, int address, int size);

private:

    void setupDefaultData();

    QModbusServer *_pModbusServer;
};

#endif // TESTSLAVEDATA_H
