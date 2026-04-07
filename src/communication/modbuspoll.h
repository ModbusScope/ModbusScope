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

    void onAdapterDiagnostic(const QString& level, const QString& message);

    /*!
     * \brief Request the adapter to construct a register expression string from its component parts.
     *
     * Delegates to AdapterClient::buildExpression(). Emits buildExpressionResult() on response.
     *
     * \param addressFields Address field values as returned by the register schema form.
     * \param dataType      Data type identifier (e.g. "16b"). Pass empty string to use the adapter default.
     * \param deviceId      Device identifier. Pass 0 to use the adapter default.
     */
    virtual void buildExpression(const QJsonObject& addressFields, const QString& dataType, deviceId_t deviceId);

signals:
    void registerDataReady(ResultDoubleList registers);

    /*!
     * \brief Emitted when an adapter.buildExpression response has been received.
     * \param expression The constructed register expression string (e.g. \c ${h0:f32b}).
     */
    void buildExpressionResult(QString expression);

private slots:
    void triggerRegisterRead();
    void onReadDataResult(ResultDoubleList results);
    void onDescribeResult(const QJsonObject& description);
    void onRegisterSchemaResult(const QJsonObject& schema);

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
