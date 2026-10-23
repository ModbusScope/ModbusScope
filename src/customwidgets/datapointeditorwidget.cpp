#include "datapointeditorwidget.h"

#include "ProtocolAdapter/datapointservice.h"
#include "customwidgets/devicecombobox.h"
#include "customwidgets/schemaformwidget.h"
#include "models/settingsmodel.h"

#include <QLabel>
#include <QSignalBlocker>
#include <QVBoxLayout>

/*!
 * \brief Constructs the editor for the first configured device and builds its default reference.
 * \param pSettingsModel    Settings model with the devices and adapter schemas.
 * \param pDataPointService Service that builds references through the device's adapter.
 * \param parent            Optional parent widget.
 */
DataPointEditorWidget::DataPointEditorWidget(SettingsModel* pSettingsModel,
                                             DataPointService* pDataPointService,
                                             QWidget* parent)
    : QWidget(parent),
      _pSettingsModel(pSettingsModel),
      _pDataPointService(pDataPointService),
      _pDeviceCombo(new DeviceComboBox(this)),
      _pProtocolLabel(new QLabel(this)),
      _pAddressForm(new SchemaFormWidget(this))
{
    auto* pLayout = new QVBoxLayout(this);
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->addWidget(_pDeviceCombo);
    pLayout->addWidget(_pProtocolLabel);
    pLayout->addWidget(_pAddressForm);

    _pDeviceCombo->setSettingsModel(_pSettingsModel);

    connect(_pDeviceCombo, &DeviceComboBox::deviceChanged, this, &DataPointEditorWidget::onDeviceSelected);
    connect(_pAddressForm, &SchemaFormWidget::valuesChanged, this, &DataPointEditorWidget::onValuesChanged);
    connect(_pDataPointService, &DataPointService::expressionBuilt, this, &DataPointEditorWidget::onExpressionBuilt);
    connect(_pDataPointService, &DataPointService::requestFailed, this, &DataPointEditorWidget::onRequestFailed);
    connect(_pDataPointService, &DataPointService::adaptersChanged, this, &DataPointEditorWidget::retryAfterFailure);
    connect(_pSettingsModel, &SettingsModel::adapterDataChanged, this, &DataPointEditorWidget::onAdapterDataChanged);

    onDeviceSelected(_pDeviceCombo->selectedDeviceId());
}

/*!
 * \brief Select a device and reset the form to its adapter's defaults.
 *
 * Builds the default reference of that adapter.
 * \param deviceId The device to select.
 */
void DataPointEditorWidget::setDevice(deviceId_t deviceId)
{
    if (_pDeviceCombo->selectedDeviceId() != deviceId)
    {
        /* Applies the device through onDeviceSelected() */
        _pDeviceCombo->setSelectedDeviceId(deviceId);
        if (_pDeviceCombo->selectedDeviceId() == deviceId)
        {
            return;
        }
    }

    onDeviceSelected(_pDeviceCombo->selectedDeviceId());
}

/*!
 * \brief Show an existing reference without rebuilding it.
 *
 * Selects the device in \c fields["deviceId"] and fills the form from \a fields. Keys the device's
 * schema does not know are ignored; missing keys take the adapter default. expression() returns
 * \a referenceText as typed until the user changes a field, so its notation is kept.
 * \param referenceText The reference as written in the expression, e.g. \c ${40002:s16b}.
 * \param fields        The \c fields of the describeDataPoint result for the reference.
 */
void DataPointEditorWidget::setReference(const QString& referenceText, const QJsonObject& fields)
{
    _pendingTicket = 0;
    _bWaitingForAdapter = false;

    const deviceId_t referenceDeviceId = static_cast<deviceId_t>(fields.value("deviceId").toInt());
    if (!_pSettingsModel->hasDevice(referenceDeviceId))
    {
        setExpression(referenceText, false);
        setError(
          QStringLiteral("Device %1 does not exist. Select a device to change the data point.").arg(referenceDeviceId));
        return;
    }

    {
        const QSignalBlocker blocker(_pDeviceCombo);
        _pDeviceCombo->setSelectedDeviceId(referenceDeviceId);
    }

    applyDevice(referenceDeviceId, fields);
    _expressionValues = _pAddressForm->values();

    setError(QString());
    setExpression(referenceText, true);
    emit deviceChanged(referenceDeviceId);
}

/*!
 * \brief Returns the selected device, or 0 when there are no devices.
 */
deviceId_t DataPointEditorWidget::deviceId() const
{
    return _pDeviceCombo->selectedDeviceId();
}

/*!
 * \brief Returns the reference set with setReference(), or the last one built from the form.
 *
 * Only usable when hasValidExpression() is true.
 */
QString DataPointEditorWidget::expression() const
{
    return _expression;
}

/*!
 * \brief Returns whether expression() matches the form: no build is pending and the last one succeeded.
 */
bool DataPointEditorWidget::hasValidExpression() const
{
    return _bValid;
}

/*!
 * \brief Reset the form to the defaults of the device's adapter and build the default reference.
 * \param deviceId The selected device; 0 when there are no devices.
 */
void DataPointEditorWidget::onDeviceSelected(deviceId_t deviceId)
{
    applyDevice(deviceId, QJsonObject());

    /* A reference of the previous device does not belong to this one */
    _pendingTicket = 0;
    setExpression(QString(), false);

    if (_pSettingsModel->hasDevice(deviceId))
    {
        requestBuild();
    }
    else
    {
        setError(QString());
    }

    emit deviceChanged(deviceId);
}

/*!
 * \brief Reset the form when the adapter of the selected device published new data, unless it holds a reference.
 * \param adapterId The adapter whose data changed.
 */
void DataPointEditorWidget::onAdapterDataChanged(const QString& adapterId)
{
    if (adapterId != _pDataPointService->adapterIdForDevice(deviceId()))
    {
        return;
    }

    retryAfterFailure();
}

/*!
 * \brief Reset the form to the defaults and build again when the last build failed because the adapter was not usable.
 *
 * Called when an adapter may have become usable. A form that failed for another reason is kept,
 * so this never discards what the user entered.
 */
void DataPointEditorWidget::retryAfterFailure()
{
    if (!_bWaitingForAdapter)
    {
        return;
    }

    onDeviceSelected(deviceId());
}

void DataPointEditorWidget::onValuesChanged()
{
    if (_bApplyingForm)
    {
        return;
    }

    /* A spin box reports its value again on Enter; that keeps the reference */
    if (_bValid && _pAddressForm->values() == _expressionValues)
    {
        return;
    }

    requestBuild();
}

/*!
 * \brief Show the reference the adapter built for the current request.
 * \param ticket     Ticket of the finished request; results of other requests are ignored.
 * \param expression The built reference.
 */
void DataPointEditorWidget::onExpressionBuilt(quint64 ticket, const QString& expression)
{
    if (ticket == 0 || ticket != _pendingTicket)
    {
        return;
    }
    _pendingTicket = 0;

    if (expression.isEmpty())
    {
        setExpression(QString(), false);
        setError(QStringLiteral("The adapter did not return a data point reference."));
        return;
    }

    setError(QString());
    setExpression(expression, true);
}

/*!
 * \brief Clear the reference and show why the current request failed.
 * \param ticket Ticket of the failed request; failures of other requests are ignored.
 * \param error  Why the request failed.
 */
void DataPointEditorWidget::onRequestFailed(quint64 ticket, const QString& error)
{
    if (ticket == 0 || ticket != _pendingTicket)
    {
        return;
    }
    _pendingTicket = 0;
    _bWaitingForAdapter = !_pDataPointService->isDeviceUsable(deviceId());

    QString reason = error.isEmpty() ? QStringLiteral("The adapter did not accept the address") : error;
    if (reason.endsWith(QLatin1Char('.')))
    {
        reason.chop(1);
    }
    setExpression(QString(), false);
    setError(QStringLiteral("%1. Change a field or select the device again to retry.").arg(reason));
}

/*!
 * \brief Rebuild the form for a device and fill it with the adapter defaults overlaid by \a fields.
 * \param deviceId The device to use; 0 when there are no devices.
 * \param fields   Field values to show instead of the defaults.
 */
void DataPointEditorWidget::applyDevice(deviceId_t deviceId, const QJsonObject& fields)
{
    const QJsonObject schema = _pDataPointService->addressSchema(deviceId);
    QJsonObject values = _pDataPointService->defaults(deviceId);

    const QJsonObject properties = schema.value("properties").toObject();
    for (auto it = fields.constBegin(); it != fields.constEnd(); ++it)
    {
        if (properties.contains(it.key()))
        {
            values.insert(it.key(), it.value());
        }
    }

    _bApplyingForm = true;
    _pAddressForm->setSchema(schema, values);
    _bApplyingForm = false;

    if (_pSettingsModel->hasDevice(deviceId))
    {
        const QString adapterId = _pDataPointService->adapterIdForDevice(deviceId);
        _pProtocolLabel->setText(QStringLiteral("Protocol: %1").arg(_pSettingsModel->adapterDisplayName(adapterId)));
    }
    else
    {
        _pProtocolLabel->clear();
    }
}

/*!
 * \brief Ask the adapter to build the reference from the form. A previous request and its error are superseded.
 */
void DataPointEditorWidget::requestBuild()
{
    _bWaitingForAdapter = false;
    _expressionValues = _pAddressForm->values();
    _pendingTicket = _pDataPointService->buildExpression(deviceId(), _expressionValues);
    setError(QString());
    setExpression(_expression, false);
}

/*!
 * \brief Store the expression and its validity and notify on change.
 */
void DataPointEditorWidget::setExpression(const QString& expression, bool bValid)
{
    const bool bChanged = expression != _expression || bValid != _bValid;
    _expression = expression;
    _bValid = bValid;

    if (bChanged)
    {
        emit expressionChanged(_expression);
    }
}

void DataPointEditorWidget::setError(const QString& error)
{
    if (error == _error)
    {
        return;
    }

    _error = error;
    emit errorChanged(_error);
}
