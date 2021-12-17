#ifndef COMMUNICATION_MANAGER_H
#define COMMUNICATION_MANAGER_H

#include <QStringListModel>
#include <QTimer>
#include "result.h"
#include "modbusregister.h"

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
    void registerDataReady(QList<Result> registers);

private slots:
    void handlePollDone(QMap<quint16, Result> resultMap, quint8 connectionId);
    void handleModbusError(QString msg);
    void handleModbusInfo(QString msg);
    void triggerRegisterRead();

private:

    QList<ModbusMasterData *> _modbusMasters;
    quint32 _activeMastersCount;

    bool _bPollActive;
    QTimer * _pPollTimer;
    qint64 _lastPollStart;

    RegisterValueHandler* _pRegisterValueHandler;

    SettingsModel * _pSettingsModel;
};

#endif // COMMUNICATION_MANAGER_H
