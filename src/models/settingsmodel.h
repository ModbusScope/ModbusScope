#ifndef SETTINGSMODEL_H
#define SETTINGSMODEL_H

#include <QObject>
#include <QDir>

class SettingsModel : public QObject
{
    Q_OBJECT
public:
    explicit SettingsModel(QObject *parent = nullptr);
    ~SettingsModel();

    void triggerUpdate(void);

    void setPollTime(quint32 pollTime);
    void setWriteDuringLogFile(QString filename);
    void setWriteDuringLogFileToDefault(void);
    void setIpAddress(quint8 connectionId, QString ip);
    void setPort(quint8 connectionId, quint16 port);
    void setSlaveId(quint8 connectionId, quint8 id);
    void setTimeout(quint8 connectionId, quint32 timeout);
    void setConsecutiveMax(quint8 connectionId, quint8 max);
    void setConnectionState(quint8 connectionId, bool bState);
    void setInt32LittleEndian(quint8 connectionId, bool int32LittleEndian);

    QString writeDuringLogFile();
    bool writeDuringLog();
    QString ipAddress(quint8 connectionId);
    quint16 port(quint8 connectionId);
    quint8 slaveId(quint8 connectionId);
    quint32 timeout(quint8 connectionId);
    quint8 consecutiveMax(quint8 connectionId);
    bool connectionState(quint8 connectionId);
    bool int32LittleEndian(quint8 connectionId);

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

    enum
    {
        CONNECTION_ID_0 = 0,
        CONNECTION_ID_1,
        CONNECTION_ID_CNT
    };

public slots:
    void setWriteDuringLog(bool bState);
    void setAbsoluteTimes(bool bAbsolute);

signals:
    void pollTimeChanged();
    void writeDuringLogChanged();
    void writeDuringLogFileChanged();
    void absoluteTimesChanged();

    void ipChanged(quint8 connectionId);
    void portChanged(quint8 connectionId);
    void slaveIdChanged(quint8 connectionId);
    void timeoutChanged(quint8 connectionId);
    void consecutiveMaxChanged(quint8 connectionId);
    void connectionStateChanged(quint8 connectionId);
    void int32LittleEndianChanged(quint8 connectionId);

private:

    typedef struct
    {
        QString ipAddress;
        quint16 port;
        quint8 slaveId;
        quint32 timeout;
        quint8 consecutiveMax;
        bool bConnectionState;
        bool bInt32LittleEndian;

    } ConnectionSettings;

    QList<ConnectionSettings> _connectionSettings;

    quint32 _pollTime;
    bool _bAbsoluteTimes;

    bool _bWriteDuringLog;
    QString _writeDuringLogFile;

};

#endif // SETTINGSMODEL_H
