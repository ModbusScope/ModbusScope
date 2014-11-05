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

    void startThread();
    void wait();

signals:
    void modbusPollDone(QList<bool> resultList, QList<quint16> registerList);
    void threadStopped();
    void modbusCommDone(quint32 success,quint32 error);

public slots:
    void readRegisterList(ModbusSettings * pSettings, QList<quint16> * pRegisterList);
    void stopThread();

private slots:
    void stopped();


private:

    modbus_t * openPort(QString ip, quint16 port);
    void closePort(modbus_t *);
    qint32 readRegisters(modbus_t * pCtx, quint16 startReg, quint32 num, QList<quint16> * pResultList);


    QThread * _thread;

};

#endif // MODBUSMASTER_H
