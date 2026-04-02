#ifndef MODBUSPOLL_H
#define MODBUSPOLL_H

#include "ProtocolAdapter/adapterclient.h"
#include "ProtocolAdapter/adapterprocess.h"
#include "communication/datapoint.h"
#include "util/result.h"

#include <QJsonObject>
#include <QStringList>
#include <QTimer>

// Forward declaration
class SettingsModel;

class ModbusPoll : public QObject
{
    Q_OBJECT
public:
    explicit ModbusPoll(SettingsModel* pSettingsModel, QObject* parent = nullptr);
    ~ModbusPoll();

    void initAdapter();
    void startCommunication(QList<DataPoint>& registerList);
    void stopCommunication();

    bool isActive();
    void resetCommunicationStats();

signals:
    void registerDataReady(ResultDoubleList registers);

private slots:
    void triggerRegisterRead();
    void onReadDataResult(ResultDoubleList results);
    void onDescribeResult(const QJsonObject& description);

private:
    QStringList buildRegisterExpressions(const QList<DataPoint>& registerList);

    QList<DataPoint> _registerList;

    bool _bPollActive;
    QTimer* _pPollTimer;
    qint64 _lastPollStart;

    SettingsModel* _pSettingsModel;
    AdapterProcess* _pAdapterProcess;
    AdapterClient* _pAdapterClient;
};

#endif // MODBUSPOLL_H
