#ifndef MODBUSPOLL_H
#define MODBUSPOLL_H

#include "ProtocolAdapter/adaptermanager.h"
#include "communication/datapoint.h"
#include "util/result.h"

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

    /*!
     * \brief Returns the AdapterManager owned by this poll instance.
     *
     * Callers that need to interact with the adapter directly (e.g. to call
     * buildExpression()) should use this accessor rather than going through ModbusPoll.
     */
    AdapterManager* adapterManager() const;

signals:
    void registerDataReady(ResultDoubleList registers);

private slots:
    void triggerRegisterRead();
    void onReadDataResult(ResultDoubleList results);

private:
    QStringList buildRegisterExpressions(const QList<DataPoint>& registerList);

    QList<DataPoint> _registerList;

    bool _bPollActive;
    QTimer* _pPollTimer;
    qint64 _lastPollStart;

    SettingsModel* _pSettingsModel;
    AdapterManager* _pAdapterManager;
};

#endif // MODBUSPOLL_H
