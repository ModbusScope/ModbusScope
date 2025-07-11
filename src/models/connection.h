#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSerialPort>

class Connection
{

public:
    explicit Connection(QObject* parent = nullptr);
    ~Connection();

    typedef enum
    {
        TYPE_TCP = 0,
        TYPE_SERIAL,
        TYPE_CNT
    } type_t;

    void setConnectionType(Connection::type_t connectionType);
    void setPortName(QString portName);
    void setParity(QSerialPort::Parity parity);
    void setBaudrate(QSerialPort::BaudRate baudrate);
    void setDatabits(QSerialPort::DataBits databits);
    void setStopbits(QSerialPort::StopBits stopbits);
    void setIpAddress(QString ip);
    void setPort(quint16 port);

    void setSlaveId(quint8 id);
    void setTimeout(quint32 timeout);
    void setConsecutiveMax(quint8 max);
    void setConnectionState(bool bState);
    void setInt32LittleEndian(bool int32LittleEndian);
    void setPersistentConnection(bool persistentConnection);

    Connection::type_t connectionType();
    QString portName();
    QSerialPort::Parity parity();
    QSerialPort::BaudRate baudrate();
    QSerialPort::DataBits databits();
    QSerialPort::StopBits stopbits();
    QString ipAddress();
    quint16 port();
    quint8 slaveId();
    quint32 timeout();
    quint8 consecutiveMax();
    bool connectionState();
    bool int32LittleEndian();
    bool persistentConnection();

    void serialConnectionStrings(QString& strParity, QString& strDataBits, QString& strStopBits);

private:
    Connection::type_t _connectionType;

    QString _ipAddress;
    quint16 _port;

    QString _portName;
    QSerialPort::Parity _parity;
    QSerialPort::BaudRate _baudrate;
    QSerialPort::DataBits _databits;
    QSerialPort::StopBits _stopbits;

    quint8 _slaveId;
    quint32 _timeout;
    quint8 _consecutiveMax;
    bool _bConnectionState;
    bool _bInt32LittleEndian;
    bool _bPersistentConnection;
};

#endif // CONNECTION_H
