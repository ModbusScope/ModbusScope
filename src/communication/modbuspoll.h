#ifndef COMMUNICATION_MANAGER_H
#define COMMUNICATION_MANAGER_H

#include "ProtocolAdapter/adapterclient.h"
#include "ProtocolAdapter/adapterprocess.h"
#include "communication/modbusregister.h"
#include "util/result.h"

#include <QJsonObject>
#include <QStringList>
#include <QTimer>

// Forward declaration
class SettingsModel;

class ModbusPoll : public QObject
{
    Q_OBJECT
public:
    explicit ModbusPoll(SettingsModel* pSettingsModel, QObject* parent = nullptr);
    ~ModbusPoll();

    void startCommunication(QList<ModbusRegister>& registerList);
    void stopCommunication();

    bool isActive();
    void resetCommunicationStats();

signals:
    void registerDataReady(ResultDoubleList registers);

private slots:
    void triggerRegisterRead();
    void onReadDataResult(ResultDoubleList results);

private:
    QJsonObject buildAdapterConfig();
    QStringList buildRegisterExpressions(const QList<ModbusRegister>& registerList);

    QList<ModbusRegister> _registerList;

    bool _bPollActive;
    QTimer* _pPollTimer;
    qint64 _lastPollStart;

    SettingsModel* _pSettingsModel;
    AdapterProcess* _pAdapterProcess;
    AdapterClient* _pAdapterClient;
};

#endif // COMMUNICATION_MANAGER_H
