#ifndef SETTINGSMODEL_H
#define SETTINGSMODEL_H

#include <QDir>
#include <QObject>

#include "models/connection.h"
#include "models/connectiontypes.h"
#include "models/device.h"

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
    void setConnectionState(ConnectionTypes::connectionId_t connectionId, bool bState);

    QString writeDuringLogFile();
    bool writeDuringLog();
    bool connectionState(ConnectionTypes::connectionId_t connectionId);
    QList<ConnectionTypes::connectionId_t> connectionList() const;
    Connection* connectionSettings(ConnectionTypes::connectionId_t connectionId);
    Device* deviceSettings(deviceId_t devId);
    quint32 pollTime();
    bool absoluteTimes();

    deviceId_t addNewDevice();
    void addDevice(deviceId_t devId);
    void removeDevice(deviceId_t devId);
    void removeAllDevice();
    bool updateDeviceId(deviceId_t oldId, deviceId_t newId);

    QList<deviceId_t> deviceList();
    QList<deviceId_t> deviceListForConnection(ConnectionTypes::connectionId_t connectionId);

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
    void deviceListChanged();

private:
    typedef struct
    {
        Connection connectionData;
        bool bConnectionState;
    } ConnectionSettings;

    QList<ConnectionSettings> _connectionSettings;
    QMap<deviceId_t, Device> _devices;

    quint32 _pollTime;
    bool _bAbsoluteTimes;

    bool _bWriteDuringLog;
    QString _writeDuringLogFile;

};

#endif // SETTINGSMODEL_H
