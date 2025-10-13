#ifndef CONNECTION_H
#define CONNECTION_H

#include "connectiontypes.h"
#include <QSerialPort>

class Connection
{

public:
    explicit Connection();
    ~Connection();

    void setConnectionType(ConnectionTypes::type_t connectionType);
    void setPortName(QString portName);
    void setParity(QSerialPort::Parity parity);
    void setBaudrate(QSerialPort::BaudRate baudrate);
    void setDatabits(QSerialPort::DataBits databits);
    void setStopbits(QSerialPort::StopBits stopbits);
    void setIpAddress(QString ip);
    void setPort(quint16 port);

    void setTimeout(quint32 timeout);
    void setPersistentConnection(bool persistentConnection);

    ConnectionTypes::type_t connectionType();
    QString portName();
    QSerialPort::Parity parity();
    QSerialPort::BaudRate baudrate();
    QSerialPort::DataBits databits();
    QSerialPort::StopBits stopbits();
    QString ipAddress();
    quint16 port();
    quint32 timeout();
    bool persistentConnection();

    void serialConnectionStrings(QString& strParity, QString& strDataBits, QString& strStopBits);

private:
    ConnectionTypes::type_t _connectionType;

    QString _ipAddress;
    quint16 _port;

    QString _portName;
    QSerialPort::Parity _parity;
    QSerialPort::BaudRate _baudrate;
    QSerialPort::DataBits _databits;
    QSerialPort::StopBits _stopbits;

    quint32 _timeout;
    bool _bPersistentConnection;
};

#endif // CONNECTION_H
