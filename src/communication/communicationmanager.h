#ifndef COMMUNICATION_MANAGER_H
#define COMMUNICATION_MANAGER_H

#include <QObject>
#include <QList>
#include <QStringListModel>

#include "modbusmaster.h"

//Forward declaration
class GuiModel;
class SettingsModel;
class GraphDataModel;
class ModbusMaster;
class QTimer;

class CommunicationManager : public QObject
{
    Q_OBJECT
public:
    explicit CommunicationManager(SettingsModel * pSettingsModel, GuiModel * pGuiModel, GraphDataModel * pGraphDataModel, QObject *parent = 0);
    ~CommunicationManager();

    bool startCommunication();
    void stopCommunication();

    bool isActive();
    void resetCommunicationStats();

public slots:
    void handlePollDone(QMap<quint16, ModbusResult> resultMap);

signals:
    void registerRequest(QList<quint16> registerList);
    void requestStop();
    void handleReceivedData(QList<bool> successList, QList<double> values);

private slots:
    void readData();
    void masterStopped();

private:

   double processValue(quint32 graphIndex, quint16 value);

    ModbusMaster * _master;
    bool _active;
    QTimer * _pPollTimer;
    qint64 _lastPollStart;

    GuiModel * _pGuiModel;
    GraphDataModel * _pGraphDataModel;
    SettingsModel * _pSettingsModel;

};

#endif // COMMUNICATION_MANAGER_H
