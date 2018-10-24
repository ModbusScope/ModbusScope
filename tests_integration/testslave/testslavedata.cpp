#include "testslavedata.h"
#include <QDebug>

TestSlaveData::TestSlaveData(QModbusServer *pModbusServer) : QObject(nullptr)
{
    /* Save reference */
    _pModbusServer = pModbusServer;

    setupDefaultData();

    connect(_pModbusServer, &QModbusServer::dataWritten, this, &TestSlaveData::onDataChanged);
}

TestSlaveData::~TestSlaveData()
{

}

void TestSlaveData::onDataChanged(QModbusDataUnit::RegisterType reg, int address, int size)
{
    qDebug() << "reg: " << reg << ", addr: " << address << ", size: " << size;
}

void TestSlaveData::setupDefaultData()
{
    QModbusDataUnitMap reg;
    const quint8 regCount = 10;
    reg.insert(QModbusDataUnit::HoldingRegisters, { QModbusDataUnit::HoldingRegisters, 0, regCount });
    _pModbusServer->setMap(reg);

    /* init data */
    for (quint8 reg = 0; reg < regCount; reg++)
    {
        _pModbusServer->setData(QModbusDataUnit::HoldingRegisters, reg, reg + 1000);
    }
}
