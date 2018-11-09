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
class ErrorLogModel;
class ModbusMaster;
class QTimer;

class CommunicationManager : public QObject
{
    Q_OBJECT
public:
    explicit CommunicationManager(SettingsModel * pSettingsModel, GuiModel * pGuiModel, GraphDataModel * pGraphDataModel, ErrorLogModel * pErrorLogModel, QObject *parent = 0);
    ~CommunicationManager();

    bool startCommunication();
    void stopCommunication();

    bool isActive();
    void resetCommunicationStats();

signals:
    void handleReceivedData(QList<bool> successList, QList<double> values);

private slots:
    void handlePollDone(QMap<quint16, ModbusResult> resultMap);
    void handleModbusError(QString msg);
    void handleModbusInfo(QString msg);
    void readData();

private:

   double processValue(quint32 graphIndex, quint16 value);

    ModbusMaster * _master;
    bool _active;
    QTimer * _pPollTimer;
    qint64 _lastPollStart;

    GuiModel * _pGuiModel;
    GraphDataModel * _pGraphDataModel;
    ErrorLogModel * _pErrorLogModel;
    SettingsModel * _pSettingsModel;

};

#endif // COMMUNICATION_MANAGER_H
