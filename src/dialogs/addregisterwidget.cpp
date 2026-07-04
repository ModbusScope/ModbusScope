#include "addregisterwidget.h"
#include "ui_addregisterwidget.h"

#include "ProtocolAdapter/adapterhub.h"
#include "ProtocolAdapter/adaptermanager.h"
#include "customwidgets/schemaformwidget.h"
#include "models/adapterdata.h"
#include "models/device.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QVBoxLayout>

/*!
 * \brief Constructs the widget and populates it from the selected adapter's register schema.
 *
 * The adapter selector is filled with all discovered adapters and hidden when
 * only a single adapter is available.
 * \param pSettingsModel Pointer to the application settings model.
 * \param pAdapterHub    Pointer to the adapter hub used to look up adapter managers.
 * \param parent         Optional parent widget.
 */
AddRegisterWidget::AddRegisterWidget(SettingsModel* pSettingsModel, AdapterHub* pAdapterHub, QWidget* parent)
    : QWidget(parent),
      _pUi(new Ui::AddRegisterWidget),
      _pAddressForm(new SchemaFormWidget(this)),
      _pSettingsModel(pSettingsModel),
      _pAdapterHub(pAdapterHub),
      _pAdapterManager(nullptr)
{
    _pUi->setupUi(this);

    _pUi->lineName->setFocus();
    _pUi->lineName->selectAll();

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto* addressLayout = new QVBoxLayout(_pUi->addressContainer);
    addressLayout->setContentsMargins(0, 0, 0, 0);
    addressLayout->addWidget(_pAddressForm);

    const QStringList adapterIds = _pAdapterHub->adapterIds();
    for (const QString& adapterId : adapterIds)
    {
        QString label = _pSettingsModel->adapterData(adapterId)->name();
        if (label.isEmpty())
        {
            label = adapterId;
        }
        _pUi->cmbAdapter->addItem(label, adapterId);
    }
    _pUi->cmbAdapter->setVisible(adapterIds.size() > 1);

    /* Connect after populating to avoid spurious slot calls during addItem */
    connect(_pUi->cmbAdapter, &QComboBox::currentIndexChanged, this, &AddRegisterWidget::onAdapterSelectionChanged);

    connect(_pUi->btnAdd, &QPushButton::clicked, this, &AddRegisterWidget::handleResultAccept);

    _axisGroup.setExclusive(true);
    _axisGroup.addButton(_pUi->radioPrimary);
    _axisGroup.addButton(_pUi->radioSecondary);

    applyAdapter(selectedAdapterId());
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
    QJsonObject schema = adapterData->dataPointSchema().value("addressSchema").toObject();

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

/*!
 * \brief Switch the widget to the given adapter's register schema.
 *
 * Rebuilds the address form from the adapter's data point schema and updates
 * the add button state based on manager and device availability.
 * \param adapterId Identifier of the adapter to use.
 */
void AddRegisterWidget::applyAdapter(const QString& adapterId)
{
    _pAdapterManager = _pAdapterHub->adapterManager(adapterId);
    if (_pAdapterManager == nullptr)
    {
        _pUi->btnAdd->setEnabled(false);
        return;
    }

    const QJsonObject dataPointSchema = _pSettingsModel->adapterData(adapterId)->dataPointSchema();
    _addressSchema = buildSchema(adapterId);
    _dataPointDefaults = dataPointSchema["defaults"].toObject();
    _pAddressForm->setSchema(_addressSchema, _dataPointDefaults);

    _pUi->btnAdd->setEnabled(!_pSettingsModel->deviceListForAdapter(adapterId).isEmpty());
}

/*!
 * \brief Returns the adapter ID of the adapter currently selected in the adapter combo box.
 */
QString AddRegisterWidget::selectedAdapterId() const
{
    return _pUi->cmbAdapter->currentData().toString();
}

/*!
 * \brief Rebuilds the address form when another adapter is selected.
 *
 * The curve name and axis selection are kept so switching adapters only
 * replaces the address fields.
 * \param index Index of the newly selected combo box entry (unused).
 */
void AddRegisterWidget::onAdapterSelectionChanged(int index)
{
    Q_UNUSED(index);
    applyAdapter(selectedAdapterId());
}

void AddRegisterWidget::handleResultAccept()
{
    if (_pAdapterManager == nullptr)
    {
        return;
    }

    collectPendingGraphData();

    QJsonObject allValues = _pAddressForm->values();
    const QString dataType = allValues.take(QStringLiteral("dataType")).toString();
    const deviceId_t deviceId =
      static_cast<deviceId_t>(allValues.take(QStringLiteral("deviceId")).toInt(Device::cFirstDeviceId));

    _pUi->btnAdd->setEnabled(false);
    connect(_pAdapterManager, &AdapterManager::buildExpressionResult, this, &AddRegisterWidget::onBuildExpressionResult,
            Qt::SingleShotConnection);
    _pAdapterManager->buildExpression(allValues, dataType, deviceId);
}

void AddRegisterWidget::onBuildExpressionResult(const QString& expression)
{
    /* Recompute instead of unconditionally enabling: the user may have switched
     * to an adapter without devices while the request was in flight */
    const bool selectedAdapterUsable =
      (_pAdapterManager != nullptr) && !_pSettingsModel->deviceListForAdapter(selectedAdapterId()).isEmpty();
    _pUi->btnAdd->setEnabled(selectedAdapterUsable);

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
