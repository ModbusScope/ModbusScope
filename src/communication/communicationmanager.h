#ifndef COMMUNICATION_MANAGER_H
#define COMMUNICATION_MANAGER_H

#include <QObject>
#include <QList>
#include <QStringListModel>

#include "registerdata.h"


//Forward declaration
class GuiModel;
class SettingsModel;
class ModbusMaster;
class QTimer;

class CommunicationManager : public QObject
{
    Q_OBJECT
public:
    explicit CommunicationManager(SettingsModel * pSettingsModel, GuiModel * pGuiModel, QObject *parent = 0);
    ~CommunicationManager();

    bool startCommunication(QList<RegisterData> registers);
    void stopCommunication();

    bool isActive();
    void resetCommunicationStats();

public slots:
    void handlePollDone(QList<bool> successList, QList<quint16> values);

signals:
    void registerRequest(QList<quint16> registerList);
    void requestStop();
    void handleReceivedData(QList<bool> successList, QList<double> values);

private slots:
    void readData();
    void masterStopped();

private:   

    ModbusMaster * _master;
    bool _active;
    QTimer * _pPollTimer;
    qint64 _lastPollStart;

    GuiModel * _pGuiModel;
    SettingsModel * _pSettingsModel;
    QList<RegisterData> _registerlist;
};

#endif // COMMUNICATION_MANAGER_H
