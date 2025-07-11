#ifndef SETTINGSMODEL_H
#define SETTINGSMODEL_H

#include <QDir>
#include <QObject>

#include "models/connection.h"
#include "models/connectiontypes.h"

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
    void setConnectionState(quint8 connectionId, bool bState);

    QString writeDuringLogFile();
    bool writeDuringLog();
    bool connectionState(quint8 connectionId);
    Connection* connectionSettings(quint8 connectionId);
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

private:
    typedef struct
    {
        Connection connectionData;
        bool bConnectionState;
    } ConnectionSettings;

    QList<ConnectionSettings> _connectionSettings;

    quint32 _pollTime;
    bool _bAbsoluteTimes;

    bool _bWriteDuringLog;
    QString _writeDuringLogFile;

};

#endif // SETTINGSMODEL_H
