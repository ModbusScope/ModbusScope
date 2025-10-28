
#include "connection.h"

Connection::Connection()
    : _ipAddress("127.0.0.1"),
      _port(502),
      _portName(QStringLiteral("COM1")),
      _parity(QSerialPort::NoParity),
      _baudrate(QSerialPort::Baud115200),
      _databits(QSerialPort::Data8),
      _stopbits(QSerialPort::OneStop),
      _timeout(1000),
      _bPersistentConnection(true)
{
    _connectionType = ConnectionTypes::TYPE_TCP;
}

Connection::~Connection()
{
}

void Connection::serialConnectionStrings(QString& strParity, QString& strDataBits, QString& strStopBits)
{
    switch (parity())
    {
    case QSerialPort::NoParity:
        strParity = QStringLiteral("no parity");
        break;
    case QSerialPort::EvenParity:
        strParity = QStringLiteral("even parity");
        break;
    case QSerialPort::OddParity:
        strParity = QStringLiteral("odd  parity");
        break;

    default:
        strParity = QStringLiteral("unknown parity");
        break;
    }

    switch (databits())
    {
    case QSerialPort::Data5:
        strDataBits = QStringLiteral("5 data bits");
        break;
    case QSerialPort::Data6:
        strDataBits = QStringLiteral("6 data bits");
        break;
    case QSerialPort::Data7:
        strDataBits = QStringLiteral("7 data bits");
        break;
    case QSerialPort::Data8:
        strDataBits = QStringLiteral("8 data bits");
        break;

    default:
        strDataBits = QStringLiteral("unknown data bits");
        break;
    }

    switch (stopbits())
    {
    case QSerialPort::OneStop:
        strStopBits = QStringLiteral("1 stop bit");
        break;
    case QSerialPort::OneAndHalfStop:
        strStopBits = QStringLiteral("1,5 stop bits");
        break;
    case QSerialPort::TwoStop:
        strStopBits = QStringLiteral("2 stop bits");
        break;

    default:
        strStopBits = QStringLiteral("unknown stop bits");
        break;
    }
}

void Connection::setPersistentConnection(bool persistentConnection)
{
    _bPersistentConnection = persistentConnection;
}

bool Connection::persistentConnection()
{
    return _bPersistentConnection;
}

void Connection::setConnectionType(ConnectionTypes::type_t connectionType)
{
    _connectionType = connectionType;
}

ConnectionTypes::type_t Connection::connectionType()
{
    return _connectionType;
}

void Connection::setPortName(QString portName)
{
    _portName = portName;
}

QString Connection::portName()
{
    return _portName;
}

void Connection::setParity(QSerialPort::Parity parity)
{
    _parity = parity;
}

QSerialPort::Parity Connection::parity()
{
    return _parity;
}

void Connection::setBaudrate(QSerialPort::BaudRate baudrate)
{
    _baudrate = baudrate;
}

QSerialPort::BaudRate Connection::baudrate()
{
    return _baudrate;
}

void Connection::setDatabits(QSerialPort::DataBits databits)
{
    _databits = databits;
}

QSerialPort::DataBits Connection::databits()
{
    return _databits;
}

void Connection::setStopbits(QSerialPort::StopBits stopbits)
{
    _stopbits = stopbits;
}

QSerialPort::StopBits Connection::stopbits()
{
    return _stopbits;
}

void Connection::setIpAddress(QString ip)
{
    _ipAddress = ip;
}

QString Connection::ipAddress()
{
    return _ipAddress;
}

void Connection::setPort(quint16 port)
{
    _port = port;
}

quint16 Connection::port()
{
    return _port;
}

quint32 Connection::timeout()
{
    return _timeout;
}

void Connection::setTimeout(quint32 timeout)
{
    _timeout = timeout;
}
