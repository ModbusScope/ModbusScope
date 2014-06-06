#ifndef MODBUSMASTER_H
#define MODBUSMASTER_H

#include <QObject>
#include "QList"

#include <modbus.h>
#include <modbussettings.h>


class ModbusMaster : public QObject
{
    Q_OBJECT
public:
    explicit ModbusMaster(QObject *parent = 0);
    virtual ~ModbusMaster();

    void StartThread();
    void Wait();

signals:
    void ReadRegisterResult(bool bSuccess, QList<quint16> pRegisterList);
    void ThreadStopped();

public slots:
    void ReadRegisterList(ModbusSettings * pSettings, QList<quint16> * pRegisterList);
    void StopThread();

private slots:
    void Stopped();


private:

    modbus_t * Connect(QString ip, quint16 port);
    void Close(modbus_t *);
    qint32 ReadRegisters(modbus_t * pCtx, quint16 startReg, quint32 num, QList<quint16> * pResultList);


    QThread * _thread;

};

#endif // MODBUSMASTER_H
