#ifndef SETTINGSMODEL_H
#define SETTINGSMODEL_H

#include <QObject>
#include <QDir>
#include <QSerialPort>

class SettingsModel : public QObject
{
    Q_OBJECT
public:

    enum
    {
        CONNECTION_ID_0 = 0,
        CONNECTION_ID_1,
        CONNECTION_ID_2,
        CONNECTION_ID_CNT
    };

    typedef enum
    {
        CONNECTION_TYPE_TCP = 0,
        CONNECTION_TYPE_SERIAL,
        CONNECTION_TYPE_CNT

    } ConnectionType_t;

    explicit SettingsModel(QObject *parent = nullptr);
    ~SettingsModel();

    void triggerUpdate(void);

    void setPollTime(quint32 pollTime);
    void setWriteDuringLogFile(QString filename);
    void setWriteDuringLogFileToDefault(void);

    void setConnectionType(quint8 connectionId, ConnectionType_t connectionType);

    void setPortName(quint8 connectionId, QString portName);
    void setParity(quint8 connectionId, QSerialPort::Parity parity);
    void setBaudrate(quint8 connectionId, QSerialPort::BaudRate baudrate);
    void setDatabits(quint8 connectionId, QSerialPort::DataBits databits);
    void setStopbits(quint8 connectionId, QSerialPort::StopBits stopbits);

    void setIpAddress(quint8 connectionId, QString ip);
    void setPort(quint8 connectionId, quint16 port);

    void setSlaveId(quint8 connectionId, quint8 id);
    void setTimeout(quint8 connectionId, quint32 timeout);
    void setConsecutiveMax(quint8 connectionId, quint8 max);
    void setConnectionState(quint8 connectionId, bool bState);
    void setInt32LittleEndian(quint8 connectionId, bool int32LittleEndian);
    void setPersistentConnection(quint8 connectionId, bool persistentConnection);

    QString writeDuringLogFile();
    bool writeDuringLog();
    ConnectionType_t connectionType(quint8 connectionId);

    QString portName(quint8 connectionId);
    QSerialPort::Parity parity(quint8 connectionId);
    QSerialPort::BaudRate baudrate(quint8 connectionId);
    QSerialPort::DataBits databits(quint8 connectionId);
    QSerialPort::StopBits stopbits(quint8 connectionId);

    QString ipAddress(quint8 connectionId);
    quint16 port(quint8 connectionId);
    quint8 slaveId(quint8 connectionId);
    quint32 timeout(quint8 connectionId);
    quint8 consecutiveMax(quint8 connectionId);
    bool connectionState(quint8 connectionId);
    bool int32LittleEndian(quint8 connectionId);
    bool persistentConnection(quint8 connectionId);

    quint32 pollTime();
    bool absoluteTimes();

    static const QString defaultLogPath()
    {
        const QString cDefaultLogFileName = "ModbusScope-autolog.csv";
        QString tempDir = QDir::toNativeSeparators(QDir::tempPath());

        if (tempDir.right(1) != QDir::separator())
        {
            tempDir.append(QDir::separator());
        }

        return tempDir.append(cDefaultLogFileName);
    }

public slots:
    void setWriteDuringLog(bool bState);
    void setAbsoluteTimes(bool bAbsolute);

signals:
    void pollTimeChanged();
    void writeDuringLogChanged();
    void writeDuringLogFileChanged();
    void absoluteTimesChanged();

    void connectionTypeChanged(quint8 connectionId);

    void portNameChanged(quint8 connectionId);
    void parityChanged(quint8 connectionId);
    void baudrateChanged(quint8 connectionId);
    void databitsChanged(quint8 connectionId);
    void stopbitsChanged(quint8 connectionId);

    void ipChanged(quint8 connectionId);
    void portChanged(quint8 connectionId);

    void slaveIdChanged(quint8 connectionId);
    void timeoutChanged(quint8 connectionId);
    void consecutiveMaxChanged(quint8 connectionId);
    void connectionStateChanged(quint8 connectionId);
    void int32LittleEndianChanged(quint8 connectionId);
    void persistentConnectionChanged(quint8 connectionId);

private:

    typedef struct
    {
        ConnectionType_t connectionType;

        QString ipAddress;
        quint16 port;

        QString portName;
        QSerialPort::Parity parity;
        QSerialPort::BaudRate baudrate;
        QSerialPort::DataBits databits;
        QSerialPort::StopBits stopbits;

        quint8 slaveId;
        quint32 timeout;
        quint8 consecutiveMax;
        bool bConnectionState;
        bool bInt32LittleEndian;
        bool bPersistentConnection;

    } ConnectionSettings;

    QList<ConnectionSettings> _connectionSettings;

    quint32 _pollTime;
    bool _bAbsoluteTimes;

    bool _bWriteDuringLog;
    QString _writeDuringLogFile;

};

#endif // SETTINGSMODEL_H
