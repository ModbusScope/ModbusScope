#ifndef MODBUSSCOPE_H
#define MODBUSSCOPE_H

#include <QObject>
#include <QList>
#include <modbussettings.h>

//Forward declaration
class ModbusMaster;
class QTimer;

class ScopeData : public QObject
{
    Q_OBJECT
public:
    explicit ScopeData(QObject *parent = 0);
    ~ScopeData();

    bool StartCommunication(ModbusSettings * pSettings, QList<quint16> * pRegisterList);
    void StopCommunication();

signals:
    void RegisterRequest(ModbusSettings * pSettings, QList<quint16> * pRegisterList);
    void RequestStop();
    void PropagateNewData(bool bSuccess, QList<quint16> values);

private slots:
    void ReadData();
    void MasterStopped();
    void ReceiveNewData(bool bSuccess, QList<quint16> values);

private:   

    ModbusMaster * _master;
    bool _active;
    QTimer * _timer;

    ModbusSettings _settings;
    QList<quint16> _registerlist;
};

#endif // MODBUSSCOPE_H
