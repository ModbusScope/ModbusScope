#ifndef MODBUSSCOPE_H
#define MODBUSSCOPE_H

#include <QObject>
#include <QList>
#include <modbussettings.h>

//Forward declaration
class ModbusMaster;
class QTimer;

class ModbusScope : public QObject
{
    Q_OBJECT
public:
    explicit ModbusScope(QObject *parent = 0);
    ~ModbusScope();

    void StartCommunication(ModbusSettings * settings, QList<quint16> * registerList);
    void StopCommunication();

signals:
    void RegisterRequest(ModbusSettings * pSettings, QList<quint16> * pRegisterList);
    void RequestStop();

public slots:
    void RegisterProcess(bool success, QList<quint16> resultList);

private slots:
    void ReadData();
    void MasterStopped();

private:   

    ModbusMaster * _master;
    bool _active;
    QTimer * _timer;

    ModbusSettings _settings;
    QList<quint16> _registerlist;
};

#endif // MODBUSSCOPE_H
