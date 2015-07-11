#ifndef MODBUSMASTER_H
#define MODBUSMASTER_H

#include <QObject>
#include "QList"

#include <modbus.h>

/* Forward declaration */
class ConnectionModel;
class GuiModel;

class ModbusMaster : public QObject
{
    Q_OBJECT
public:
    explicit ModbusMaster(ConnectionModel * pConnectionModel, GuiModel *pGuiModel, QObject *parent = 0);
    virtual ~ModbusMaster();

    void startThread();
    void wait();

signals:
    void modbusPollDone(QList<bool> resultList, QList<quint16> registerList);
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

    ConnectionModel * _pConnectionModel;
    GuiModel * _pGuiModel;
    QThread * _pThread;

    static const quint32 _cMaxRegistersInOneRead = 20;

};

#endif // MODBUSMASTER_H
