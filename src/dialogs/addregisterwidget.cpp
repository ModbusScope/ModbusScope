#include "addregisterwidget.h"
#include "ui_addregisterwidget.h"

#include "ProtocolAdapter/adaptermanager.h"
#include "customwidgets/schemaformwidget.h"
#include "models/adapterdata.h"
#include "models/device.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QVBoxLayout>

/*!
 * \brief Constructs the widget and populates it from the adapter's register schema.
 * \param pSettingsModel Pointer to the application settings model.
 * \param adapterId      Identifier of the adapter whose register schema to use.
 * \param pAdapterManager Pointer to the adapter manager used to build expression strings.
 * \param parent         Optional parent widget.
 */
AddRegisterWidget::AddRegisterWidget(SettingsModel* pSettingsModel,
                                     const QString& adapterId,
                                     AdapterManager* pAdapterManager,
                                     QWidget* parent)
    : QWidget(parent),
      _pUi(new Ui::AddRegisterWidget),
      _pAddressForm(new SchemaFormWidget(this)),
      _pSettingsModel(pSettingsModel),
      _pAdapterManager(pAdapterManager)
{
    _pUi->setupUi(this);

    _pUi->lineName->setFocus();
    _pUi->lineName->selectAll();

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    const QJsonObject dataPointSchema = pSettingsModel->adapterData(adapterId)->dataPointSchema();
    _addressSchema = buildSchema(adapterId);
    _dataPointDefaults = dataPointSchema["defaults"].toObject();

    auto* addressLayout = new QVBoxLayout(_pUi->addressContainer);
    addressLayout->setContentsMargins(0, 0, 0, 0);
    addressLayout->addWidget(_pAddressForm);

    if (_pSettingsModel->deviceListForAdapter(adapterId).isEmpty())
    {
        _pUi->btnAdd->setEnabled(false);
    }

    connect(_pUi->btnAdd, &QPushButton::clicked, this, &AddRegisterWidget::handleResultAccept);
    connect(_pAdapterManager, &AdapterManager::buildExpressionResult, this,
            &AddRegisterWidget::onBuildExpressionResult);

    _axisGroup.setExclusive(true);
    _axisGroup.addButton(_pUi->radioPrimary);
    _axisGroup.addButton(_pUi->radioSecondary);

    resetFields();
}

AddRegisterWidget::~AddRegisterWidget()
{
    delete _pUi;
}

/*!
 * \brief Build the address schema patched with available devices as a deviceId enum.
 * \param adapterId Adapter whose data point schema and device list to use.
 * \return The patched schema object ready for SchemaFormWidget.
 */
QJsonObject AddRegisterWidget::buildSchema(const QString& adapterId) const
{
    const AdapterData* adapterData = _pSettingsModel->adapterData(adapterId);
    QJsonObject schema = adapterData->dataPointSchema()["addressSchema"].toObject();

    const auto deviceList = _pSettingsModel->deviceListForAdapter(adapterId);
    QJsonArray deviceEnum;
    QJsonArray deviceLabels;
    for (deviceId_t devId : deviceList)
    {
        deviceEnum.append(static_cast<int>(devId));
        deviceLabels.append(tr("Device %1").arg(devId));
    }

    QJsonObject propsObj = schema.value("properties").toObject();
    if (propsObj.contains(QStringLiteral("deviceId")))
    {
        QJsonObject deviceIdProp = propsObj.value("deviceId").toObject();
        deviceIdProp["enum"] = deviceEnum;
        deviceIdProp["x-enumLabels"] = deviceLabels;
        propsObj["deviceId"] = deviceIdProp;
        schema["properties"] = propsObj;
    }

    return schema;
}

void AddRegisterWidget::handleResultAccept()
{
    collectPendingGraphData();

    QJsonObject allValues = _pAddressForm->values();
    const QString dataType = allValues.take(QStringLiteral("dataType")).toString();
    const deviceId_t deviceId =
      static_cast<deviceId_t>(allValues.take(QStringLiteral("deviceId")).toInt(Device::cFirstDeviceId));

    _pUi->btnAdd->setEnabled(false);
    _pAdapterManager->buildExpression(allValues, dataType, deviceId);
}

void AddRegisterWidget::onBuildExpressionResult(const QString& expression)
{
    _pUi->btnAdd->setEnabled(true);

    if (expression.isEmpty())
    {
        return;
    }

    _pendingGraphData.setExpression(expression);
    emit graphDataConfigured(_pendingGraphData);

    resetFields();
}

void AddRegisterWidget::resetFields()
{
    _pUi->lineName->setText("Name of curve");
    _pUi->radioPrimary->setChecked(true);
    _pAddressForm->setSchema(_addressSchema, _dataPointDefaults);
}

/*!
 * \brief Captures the current non-expression fields into \a _pendingGraphData.
 *
 * Called just before the async adapter.buildExpression request is sent, so that
 * the label and value axis are snapshotted at click time.
 */
void AddRegisterWidget::collectPendingGraphData()
{
    _pendingGraphData = GraphData();
    _pendingGraphData.setLabel(_pUi->lineName->text());

    if (_pUi->radioSecondary->isChecked())
    {
        _pendingGraphData.setValueAxis(GraphData::VALUE_AXIS_SECONDARY);
    }
    else
    {
        _pendingGraphData.setValueAxis(GraphData::VALUE_AXIS_PRIMARY);
    }
}
