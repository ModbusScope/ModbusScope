#ifndef SETTINGSMODEL_H
#define SETTINGSMODEL_H

#include <QObject>

class SettingsModel : public QObject
{
    Q_OBJECT
public:
    explicit SettingsModel(QObject *parent = 0);
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
    quint32 writeDuringLog();
    QString ipAddress();
    quint16 port();
    quint8 slaveId();
    quint32 timeout();
    quint32 pollTime();
    bool absoluteTimes();
    quint8 consecutiveMax();

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

    QString _ipAddress;
    quint16 _port;
    quint8 _slaveId;
    quint32 _timeout;
    quint32 _pollTime;
    quint8 _consecutiveMax;

    bool _bAbsoluteTimes;

    bool _bWriteDuringLog;
    QString _writeDuringLogFile;

    QString defaultLogFilePath;

    static const QString _cDefaultLogFileName;

};

#endif // SETTINGSMODEL_H
