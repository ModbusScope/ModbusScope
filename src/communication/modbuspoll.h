#ifndef COMMUNICATION_MANAGER_H
#define COMMUNICATION_MANAGER_H

#include "communication/modbusregister.h"
#include "util/modbusresultmap.h"
#include <QStringListModel>
#include <QTimer>

//Forward declaration
class SettingsModel;
class RegisterValueHandler;
class ModbusMaster;

class ModbusMasterData : public QObject
{
    Q_OBJECT
public:

    explicit ModbusMasterData(ModbusMaster * pArgModbusMaster, QObject *parent = nullptr):
        QObject(parent)
    {
        pModbusMaster = pArgModbusMaster;
        bActive = false;
    }

    ModbusMaster * pModbusMaster;
    bool bActive;
};

class ModbusPoll : public QObject
{
    Q_OBJECT
public:
    explicit ModbusPoll(SettingsModel * pSettingsModel, QObject *parent = nullptr);
    ~ModbusPoll();

    void startCommunication(QList<ModbusRegister>& registerList);
    void stopCommunication();

    bool isActive();
    void resetCommunicationStats();

signals:
    void registerDataReady(ResultDoubleList registers);

private slots:
    void handlePollDone(ModbusResultMap partialResultMap, connectionId_t connectionId);
    void handleModbusError(QString msg);
    void handleModbusInfo(QString msg);
    void triggerRegisterRead();

private:
    quint8 lowestConsecutiveMaxForConnection(connectionId_t connId) const;

    QList<ModbusMasterData*> _modbusMasters;
    quint32 _activeMastersCount;

    bool _bPollActive;
    QTimer * _pPollTimer;
    qint64 _lastPollStart;

    RegisterValueHandler* _pRegisterValueHandler;

    SettingsModel * _pSettingsModel;
};

#endif // COMMUNICATION_MANAGER_H
