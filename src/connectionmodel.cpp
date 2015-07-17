
#include "connectionmodel.h"


ConnectionModel::ConnectionModel(QObject *parent) :
    QObject(parent)
{
    clearData();
}

ConnectionModel::~ConnectionModel()
{

}

void ConnectionModel::clearData()
{
   setIpAddress("127.0.0.1");
   setPort(502);
   setSlaveId(1);
   setTimeout(1000);
}

void ConnectionModel::setIpAddress(QString ip)
{
    if (_ipAddress != ip)
    {
        _ipAddress = ip;
        emit ipChanged();
    }
}

QString ConnectionModel::ipAddress()
{
    return _ipAddress;
}

void ConnectionModel::setPort(quint16 port)
{
    if (_port != port)
    {
        _port = port;
        emit portChanged();
    }
}

quint16 ConnectionModel::port()
{
    return _port;
}

quint8 ConnectionModel::slaveId()
{
    return _slaveId;
}

void ConnectionModel::setSlaveId(quint8 id)
{
    if (_slaveId != id)
    {
        _slaveId = id;
        emit slaveIdChanged();
    }
}

quint32 ConnectionModel::timeout()
{
    return _timeout;
}

void ConnectionModel::setTimeout(quint32 timeout)
{
    if (_timeout != timeout)
    {
        _timeout = timeout;
        emit timeoutChanged();
    }
}
