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

    bool startCommunication(ModbusSettings * pSettings, QList<quint16> registers);
    void stopCommunication();
    qint64 getCommunicationStartTime();
    qint64 getCommunicationEndTime();
    void getCommunicationSettings(quint32 * successCount, quint32 * errorCount);

public slots:
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
    qint64 _startTime;
    qint64 _endTime;

    qint64 _lastPollStart;

    ModbusSettings _settings;
    QList<quint16> _registerlist;
};

#endif // MODBUSSCOPE_H
