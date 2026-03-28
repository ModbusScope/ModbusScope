#ifndef SETTINGSMODEL_H
#define SETTINGSMODEL_H

#include <QDir>
#include <QObject>

#include "models/adapterdata.h"
#include "models/device.h"

class SettingsModel : public QObject
{
    Q_OBJECT
public:
    explicit SettingsModel(QObject* parent = nullptr);
    ~SettingsModel();

    void triggerUpdate(void);

    void setPollTime(quint32 pollTime);
    void setWriteDuringLogFile(QString filename);
    void setWriteDuringLogFileToDefault(void);

    QString writeDuringLogFile();
    bool writeDuringLog();
    Device* deviceSettings(deviceId_t devId);
    quint32 pollTime();
    bool absoluteTimes();

    deviceId_t addNewDevice();
    void addDevice(deviceId_t devId);
    void removeDevice(deviceId_t devId);
    void removeAllDevice();
    bool updateDeviceId(deviceId_t oldId, deviceId_t newId);
    bool hasDevice(deviceId_t devId) const;

    QList<deviceId_t> deviceList();
    QList<deviceId_t> deviceListForAdapter(const QString& adapterId);

    const AdapterData* adapterData(const QString& adapterId);
    QStringList adapterIds() const;
    void removeAdapter(const QString& adapterId);

    void setAdapterCurrentConfig(const QString& adapterId, const QJsonObject& config);
    void updateAdapterFromDescribe(const QString& adapterId, const QJsonObject& describeResult);

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
    void adapterDataChanged(const QString& adapterId);

private:
    QMap<deviceId_t, Device> _devices;
    QMap<QString, AdapterData> _adapters;

    quint32 _pollTime;
    bool _bAbsoluteTimes;

    bool _bWriteDuringLog;
    QString _writeDuringLogFile;
};

#endif // SETTINGSMODEL_H
