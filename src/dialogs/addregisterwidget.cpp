#include "addregisterwidget.h"
#include "ui_addregisterwidget.h"

#include "customwidgets/schemaformwidget.h"
#include "models/adapterdata.h"
#include "models/device.h"
#include "models/settingsmodel.h"
#include "util/expressiongenerator.h"

#include <QJsonArray>
#include <QVBoxLayout>

AddRegisterWidget::AddRegisterWidget(SettingsModel* pSettingsModel, const QString& adapterId, QWidget* parent)
    : QWidget(parent),
      _pUi(new Ui::AddRegisterWidget),
      _pAddressForm(new SchemaFormWidget(this)),
      _pSettingsModel(pSettingsModel)
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

    /* Pre-select the default data type */
    const int defaultIndex = _pUi->cmbType->findData(defaultTypeId);
    if (defaultIndex >= 0)
    {
        _pUi->cmbType->setCurrentIndex(defaultIndex);
    }

    /* Populate device combo */
    _pUi->cmbDevice->clear();
    const auto deviceList = _pSettingsModel->deviceListForAdapter(adapterId);
    for (deviceId_t devId : std::as_const(deviceList))
    {
        _pUi->cmbDevice->addItem(QString(tr("Device %1").arg(devId)), devId);
    }

    connect(_pUi->btnAdd, &QPushButton::clicked, this, &AddRegisterWidget::handleResultAccept);

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
    QString expression = generateExpression();
    GraphData graphData;

    graphData.setLabel(_pUi->lineName->text());

    if (_pUi->radioSecondary->isChecked())
    {
        graphData.setValueAxis(GraphData::VALUE_AXIS_SECONDARY);
    }
    else
    {
        graphData.setValueAxis(GraphData::VALUE_AXIS_PRIMARY);
    }

    graphData.setExpression(expression);

    emit graphDataConfigured(graphData);

    resetFields();
}

void AddRegisterWidget::resetFields()
{
    _pUi->lineName->setText("Name of curve");
    _pUi->cmbType->setCurrentIndex(0);
    _pUi->cmbDevice->setCurrentIndex(0);
    _pUi->radioPrimary->setChecked(true);
    _pAddressForm->setSchema(_addressSchema, QJsonObject());
}

QString AddRegisterWidget::generateExpression()
{
    const QJsonObject addressValues = _pAddressForm->values();
    const QString objectType = addressValues["objectType"].toString();
    const int address = addressValues["address"].toInt();

    const QString typeId = _pUi->cmbType->currentData().toString();

    deviceId_t deviceId = Device::cFirstDeviceId;
    const QVariant devData = _pUi->cmbDevice->currentData();
    if (devData.canConvert<deviceId_t>())
    {
        deviceId = devData.value<deviceId_t>();
    }

    return ExpressionGenerator::constructRegisterString(objectType, address, typeId, deviceId);
}
