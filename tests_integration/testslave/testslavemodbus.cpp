#include "testslavemodbus.h"
#include <QDebug>

TestSlaveModbus::TestSlaveModbus(QObject *parent) : QModbusTcpServer(parent)
{

}

TestSlaveModbus::~TestSlaveModbus()
{

}

bool TestSlaveModbus::connect(QUrl host, int slaveId)
{
    setConnectionParameter(QModbusDevice::NetworkPortParameter, host.port());
    setConnectionParameter(QModbusDevice::NetworkAddressParameter, host.host());
    setServerAddress(slaveId);

    return connectDevice();
}
