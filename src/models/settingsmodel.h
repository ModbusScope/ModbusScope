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
    void setIpAddress(QString ip);
    void setPort(quint16 port);
    void setSlaveId(quint8 id);
    void setTimeout(quint32 timeout);
    void setConsecutiveMax(quint8 max);

    QString writeDuringLogFile();
    bool writeDuringLog();
    QString ipAddress(); // TODO: remove
    QString ipAddress(quint8 connectionId);
    quint16 port(); // TODO: remove
    quint16 port(quint8 connectionId);
    quint8 slaveId(); // TODO: remove
    quint8 slaveId(quint8 connectionId);
    quint32 timeout(); // TODO: remove
    quint32 timeout(quint8 connectionId);

    quint32 pollTime();
    bool absoluteTimes();

     // TODO: add connectionId arguments
    quint8 consecutiveMax();

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
    void ipChanged();
    void portChanged();
    void slaveIdChanged();
    void timeoutChanged();
    void absoluteTimesChanged();
    void consecutiveMaxChanged();

private:

    typedef struct
    {
        QString ipAddress;
        quint16 port;
        quint8 slaveId;
        quint32 timeout;
        quint8 consecutiveMax;

    } ConnectionSettings;

    enum
    {
        CONNECTION_ID_0 = 0,
        CONNECTION_ID_1,
        CONNECTION_ID_CNT
    };

    QList<ConnectionSettings> _connectionSettings;

    quint32 _pollTime;
    bool _bAbsoluteTimes;

    bool _bWriteDuringLog;
    QString _writeDuringLogFile;

};

#endif // SETTINGSMODEL_H
