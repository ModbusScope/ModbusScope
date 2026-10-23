#ifndef DATAPOINTEDITORWIDGET_H
#define DATAPOINTEDITORWIDGET_H

#include "models/device.h"

#include <QJsonObject>
#include <QString>
#include <QWidget>

class DataPointService;
class DeviceComboBox;
class QLabel;
class SchemaFormWidget;
class SettingsModel;

class DataPointEditorWidget : public QWidget
{
    Q_OBJECT

    friend class TestDataPointEditorWidget;

public:
    explicit DataPointEditorWidget(SettingsModel* pSettingsModel,
                                   DataPointService* pDataPointService,
                                   QWidget* parent = nullptr);

    void setDevice(deviceId_t deviceId);
    void setReference(const QString& referenceText, const QJsonObject& fields);

    deviceId_t deviceId() const;
    QString expression() const;
    bool hasValidExpression() const;

signals:
    void expressionChanged(const QString& expression);
    void errorChanged(const QString& error);
    void deviceChanged(deviceId_t deviceId);

private slots:
    void onDeviceSelected(deviceId_t deviceId);
    void onValuesChanged();
    void onExpressionBuilt(quint64 ticket, const QString& expression);
    void onRequestFailed(quint64 ticket, const QString& error);
    void onAdapterDataChanged(const QString& adapterId);
    void retryAfterFailure();

private:
    void applyDevice(deviceId_t deviceId, const QJsonObject& fields);
    void requestBuild();
    void setExpression(const QString& expression, bool bValid);
    void setError(const QString& error);

    SettingsModel* _pSettingsModel;
    DataPointService* _pDataPointService;

    DeviceComboBox* _pDeviceCombo;
    QLabel* _pProtocolLabel;
    SchemaFormWidget* _pAddressForm;

    QString _expression;
    bool _bValid{ false };

    //! Form values that _expression was set or built from.
    QJsonObject _expressionValues;
    QString _error;
    quint64 _pendingTicket{ 0 };

    //! Set when the last build failed because the adapter was not usable; it is retried when that changes.
    bool _bWaitingForAdapter{ false };

    //! Set while the form is filled in code, so its valuesChanged() does not count as a user edit.
    bool _bApplyingForm{ false };
};

#endif // DATAPOINTEDITORWIDGET_H
