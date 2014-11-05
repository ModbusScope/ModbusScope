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
    void getRegisterList(QList<quint16> * pList);
    void clearRegisterList();

public slots:
    void toggleRegister(quint16 registerAddress);
    void removedRegister(quint16 registerAddress);
    void processCommStats(quint32 success,quint32 error);
    void handlePollDone(QList<bool> successList, QList<quint16> values);

signals:
    void registerRequest(ModbusSettings * pSettings, QList<quint16> * pRegisterList);
    void requestStop();
    void handleReceivedData(QList<bool> successList, QList<quint16> values);
    void triggerStatUpdate(quint32 successCount, quint32 errorCount);

private slots:
    void readData();
    void masterStopped();
private:   

    ModbusMaster * _master;
    bool _active;
    QTimer * _timer;

    quint32 _successCount;
    quint32 _errorCount;

    ModbusSettings _settings;
    QList<quint16> _registerlist;
};

#endif // MODBUSSCOPE_H
