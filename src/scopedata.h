#ifndef MODBUSSCOPE_H
#define MODBUSSCOPE_H

#include <QObject>
#include <QList>
#include <modbussettings.h>
#include <QStringListModel>

//Forward declaration
class ModbusMaster;
class QTimer;

class ScopeData : public QObject
{
    Q_OBJECT
public:
    explicit ScopeData(QObject *parent = 0);
    ~ScopeData();

    bool startCommunication(ModbusSettings * pSettings);
    void stopCommunication();
    quint32 getRegisterCount();

public slots:
    void toggleRegister(quint16 registerAddress);
    void removedRegister(quint16 registerAddress);

signals:
    void registerRequest(ModbusSettings * pSettings, QList<quint16> * pRegisterList);
    void requestStop();
    void propagateNewData(bool bSuccess, QList<quint16> values);

private slots:
    void readData();
    void masterStopped();
    void receiveNewData(bool bSuccess, QList<quint16> values);

private:   

    ModbusMaster * _master;
    bool _active;
    QTimer * _timer;

    ModbusSettings _settings;
    QList<quint16> _registerlist;
};

#endif // MODBUSSCOPE_H
