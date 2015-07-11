#ifndef COMMUNICATION_MANAGER_H
#define COMMUNICATION_MANAGER_H

#include <QObject>
#include <QList>
#include <QStringListModel>

#include "connectionmodel.h"
#include "registerdata.h"


//Forward declaration
class GuiModel;
class ModbusMaster;
class QTimer;

class CommunicationManager : public QObject
{
    Q_OBJECT
public:
    explicit CommunicationManager(ConnectionModel * pConnectionModel, GuiModel * pGuiModel, QObject *parent = 0);
    ~CommunicationManager();

    bool startCommunication(QList<RegisterData> registers);
    void stopCommunication();

    void communicationSettings(quint32 * successCount, quint32 * errorCount);
    bool isActive();
    void resetCommunicationStats();

public slots:
    void processCommStats(quint32 success,quint32 error);
    void handlePollDone(QList<bool> successList, QList<quint16> values);

signals:
    void registerRequest(QList<quint16> registerList);
    void requestStop();
    void handleReceivedData(QList<bool> successList, QList<double> values);
    void triggerStatUpdate(quint32 successCount, quint32 errorCount);

private slots:
    void readData();
    void masterStopped();

private:   

    ModbusMaster * _master;
    bool _active;
    QTimer * _pPollTimer;

    quint32 _successCount; // TODO: move to model
    quint32 _errorCount; // TODO: move to model

    qint64 _lastPollStart;

    GuiModel * _pGuiModel;
    ConnectionModel * _pConnectionModel;
    QList<RegisterData> _registerlist;
};

#endif // COMMUNICATION_MANAGER_H
