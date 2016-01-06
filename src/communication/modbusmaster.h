#ifndef MODBUSMASTER_H
#define MODBUSMASTER_H

#include <QObject>
#include <QMap>
#include "modbusresult.h"

#include <modbus.h>

/* Forward declaration */
class SettingsModel;
class GuiModel;

class ModbusMaster : public QObject
{
    Q_OBJECT
public:
    explicit ModbusMaster(SettingsModel * pSettingsModel, GuiModel *pGuiModel, QObject *parent = 0);
    virtual ~ModbusMaster();

    void startThread();
    void wait();

signals:
    void modbusPollDone(QMap<quint16, ModbusResult> modbusResults);
    void threadStopped();

public slots:
    void readRegisterList(QList<quint16> registerList);
    void stopThread();

private slots:
    void stopped();


private:

    modbus_t * openPort(QString ip, quint16 port);
    void closePort(modbus_t *);
    qint32 readRegisters(modbus_t * pCtx, quint16 startReg, quint32 num, QList<quint16> * pResultList);

    SettingsModel * _pSettingsModel;
    GuiModel * _pGuiModel;
    QThread * _pThread;

};

#endif // MODBUSMASTER_H
