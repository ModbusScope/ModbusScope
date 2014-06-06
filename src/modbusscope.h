#ifndef MODBUSSCOPE_H
#define MODBUSSCOPE_H

#include <QObject>
#include <QList>
#include <modbussettings.h>

//Forward declaration
class ModbusMaster;
class ScopeGui;
class QCustomPlot;
class QTimer;

class ModbusScope : public QObject
{
    Q_OBJECT
public:
    explicit ModbusScope(QCustomPlot * pGraph, QObject *parent = 0);
    ~ModbusScope();

    void StartCommunication(ModbusSettings * pSettings, QList<quint16> * pRegisterList);
    void StopCommunication();

signals:
    void RegisterRequest(ModbusSettings * pSettings, QList<quint16> * pRegisterList);
    void RequestStop();

private slots:
    void ReadData();
    void MasterStopped();

private:   

    ModbusMaster * _master;
    ScopeGui * _gui;
    bool _active;
    QTimer * _timer;

    ModbusSettings _settings;
    QList<quint16> _registerlist;
};

#endif // MODBUSSCOPE_H
