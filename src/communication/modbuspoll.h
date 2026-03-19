#ifndef COMMUNICATION_MANAGER_H
#define COMMUNICATION_MANAGER_H

#include "communication/modbusregister.h"
#include "util/result.h"
#include <QTimer>

//Forward declaration
class SettingsModel;

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
    void triggerRegisterRead();

private:
    QList<ModbusRegister> _registerList;

    bool _bPollActive;
    QTimer * _pPollTimer;
    qint64 _lastPollStart;

    SettingsModel * _pSettingsModel;
};

#endif // COMMUNICATION_MANAGER_H
