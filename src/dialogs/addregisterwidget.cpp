#include "addregisterwidget.h"
#include "ui_addregisterwidget.h"

#include "communication/modbuspoll.h"
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
 * \param pModbusPoll    Pointer to the Modbus poll instance used to build expression strings.
 * \param parent         Optional parent widget.
 */
AddRegisterWidget::AddRegisterWidget(SettingsModel* pSettingsModel,
                                     const QString& adapterId,
                                     ModbusPoll* pModbusPoll,
                                     QWidget* parent)
    : QWidget(parent),
      _pUi(new Ui::AddRegisterWidget),
      _pAddressForm(new SchemaFormWidget(this)),
      _pSettingsModel(pSettingsModel),
      _pModbusPoll(pModbusPoll)
{
    _pUi->setupUi(this);

    _pUi->lineName->setFocus();
    _pUi->lineName->selectAll();

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    /* Build the address form from the adapter's register schema */
    const AdapterData* adapterData = _pSettingsModel->adapterData(adapterId);
    const QJsonObject registerSchema = adapterData->registerSchema();
    _addressSchema = registerSchema["addressSchema"].toObject();
    _pAddressForm->setSchema(_addressSchema, QJsonObject());

    auto* addressLayout = new QVBoxLayout(_pUi->addressContainer);
    addressLayout->setContentsMargins(0, 0, 0, 0);
    addressLayout->addWidget(_pAddressForm);

    /* Populate data type combo from the adapter's dataTypes array */
    const QJsonArray dataTypes = registerSchema["dataTypes"].toArray();
    const QString defaultTypeId = registerSchema["defaultDataType"].toString();
    for (const QJsonValue& entry : dataTypes)
    {
        const QJsonObject typeObj = entry.toObject();
        _pUi->cmbType->addItem(typeObj["label"].toString(), typeObj["id"].toString());
    }

    /* Pre-select the default data type and remember the index for resetFields() */
    _defaultTypeIndex = _pUi->cmbType->findData(defaultTypeId);
    if (_defaultTypeIndex < 0)
    {
        _defaultTypeIndex = 0;
    }
    _pUi->cmbType->setCurrentIndex(_defaultTypeIndex);

    /* Populate device combo */
    _pUi->cmbDevice->clear();
    const auto deviceList = _pSettingsModel->deviceListForAdapter(adapterId);
    for (deviceId_t devId : std::as_const(deviceList))
    {
        _pUi->cmbDevice->addItem(QString(tr("Device %1").arg(devId)), devId);
    }

    if (deviceList.isEmpty())
    {
        _pUi->btnAdd->setEnabled(false);
        _pUi->cmbDevice->setEnabled(false);
    }

    connect(_pUi->btnAdd, &QPushButton::clicked, this, &AddRegisterWidget::handleResultAccept);
    connect(_pModbusPoll, &ModbusPoll::buildExpressionResult, this, &AddRegisterWidget::onBuildExpressionResult);

    _axisGroup.setExclusive(true);
    _axisGroup.addButton(_pUi->radioPrimary);
    _axisGroup.addButton(_pUi->radioSecondary);

    resetFields();
}

AddRegisterWidget::~AddRegisterWidget()
{
    delete _pUi;
}

void AddRegisterWidget::handleResultAccept()
{
    if (_pUi->cmbDevice->count() == 0)
    {
        return;
    }

    collectPendingGraphData();

    const QJsonObject addressValues = _pAddressForm->values();
    const QString typeId = _pUi->cmbType->currentData().toString();

    deviceId_t deviceId = Device::cFirstDeviceId;
    const QVariant devData = _pUi->cmbDevice->currentData();
    if (devData.canConvert<deviceId_t>())
    {
        deviceId = devData.value<deviceId_t>();
    }

    _pUi->btnAdd->setEnabled(false);
    _pModbusPoll->buildExpression(addressValues, typeId, deviceId);
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
    _pUi->cmbType->setCurrentIndex(_defaultTypeIndex);
    _pUi->cmbDevice->setCurrentIndex(0);
    _pUi->radioPrimary->setChecked(true);
    _pAddressForm->setSchema(_addressSchema, QJsonObject());
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
