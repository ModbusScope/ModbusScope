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
    void setWriteDuringLogPath(QString path);    
    void setIpAddress(QString ip);
    void setPort(quint16 port);
    void setSlaveId(quint8 id);
    void setTimeout(quint32 timeout);

    QString writeDuringLogPath();
    quint32 writeDuringLog();
    QString ipAddress();
    quint16 port();
    quint8 slaveId();
    quint32 timeout();
    quint32 pollTime();

public slots:
    void setWriteDuringLog(bool bState);

signals:
    void pollTimeChanged();
    void writeDuringLogChanged();
    void writeDuringLogPathChanged();
    void ipChanged();
    void portChanged();
    void slaveIdChanged();
    void timeoutChanged();

private:

    QString _ipAddress;
    quint16 _port;
    quint8 _slaveId;
    quint32 _timeout;
    quint32 _pollTime;

    bool _bWriteDuringLog;
    QString _writeDuringLogPath;

};

#endif // SETTINGSMODEL_H
