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
    void modbusLogError(QString msg);
    void modbusLogInfo(QString msg);
    void threadStopped();

public slots:
    void readRegisterList(QList<quint16> registerList);
    void stopThread();

private slots:
    void stopped();


private:

    bool openPort();
    void closePort(modbus_t *);
    qint32 readRegisters(modbus_t * pCtx, quint16 startReg, quint32 num, QList<quint16> * pResultList);
    void readPartialList(QList<quint16> registerList);
    QList<quint16> getSubsequentRegisterList(QList<quint16> registerList);
    void addSuccessResults(QList<quint16> registerList, QList<quint16> registerDataList);
    void addErrorResults(QList<quint16> registerList);
    QString dumpToString(QMap<quint16, ModbusResult> map);
    QString dumpToString(QList<quint16> list);

    modbus_t * _pModbusContext;

    bool _bOpen;
    quint32 _success;
    quint32 _error;
    QMap<quint16, ModbusResult> _resultMap;

    SettingsModel * _pSettingsModel;
    GuiModel * _pGuiModel;
    QThread * _pThread;

};

#endif // MODBUSMASTER_H
