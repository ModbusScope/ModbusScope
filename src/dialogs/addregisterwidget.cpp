#include "addregisterwidget.h"
#include "ui_addregisterwidget.h"

#include "ProtocolAdapter/adapterhub.h"
#include "ProtocolAdapter/adaptermanager.h"
#include "customwidgets/schemaformwidget.h"
#include "models/adapterdata.h"
#include "models/device.h"
#include "models/settingsmodel.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QMenu>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QVBoxLayout>

/*!
 * \brief Constructs the widget and populates it from the selected device's register schema.
 *
 * The device selector is filled with all configured devices.
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

    populateDeviceCombo();

    /* Connect after populating to avoid spurious slot calls during addItem */
    connect(_pUi->cmbDevice, &QComboBox::currentIndexChanged, this, &AddRegisterWidget::onDeviceSelectionChanged);
    connect(_pSettingsModel, &SettingsModel::deviceListChanged, this, &AddRegisterWidget::refreshDeviceCombo);

    connect(_pUi->btnAdd, &QPushButton::clicked, this, &AddRegisterWidget::handleResultAccept);

    _axisGroup.setExclusive(true);
    _axisGroup.addButton(_pUi->radioPrimary);
    _axisGroup.addButton(_pUi->radioSecondary);

    applyDevice(selectedDeviceId());
    resetFields();
}

/*!
 * \brief Clears and refills the device combo box with all configured devices.
 */
void AddRegisterWidget::populateDeviceCombo()
{
    _pUi->cmbDevice->clear();

    const QList<deviceId_t> deviceIds = _pSettingsModel->deviceList();
    for (deviceId_t devId : deviceIds)
    {
        _pUi->cmbDevice->addItem(_pSettingsModel->deviceSettings(devId)->name(), QVariant(devId));
    }
}

/*!
 * \brief Repopulates the device combo box, preserving the current selection where possible.
 *
 * Signals are blocked while the combo box is cleared and rebuilt so the transient,
 * possibly-empty intermediate states it passes through don't reach applyDevice();
 * the device is applied exactly once, for the final selection.
 */
void AddRegisterWidget::refreshDeviceCombo()
{
    const deviceId_t previousSelection = selectedDeviceId();

    {
        const QSignalBlocker blocker(_pUi->cmbDevice);
        populateDeviceCombo();

        const int idx = _pUi->cmbDevice->findData(QVariant(previousSelection));
        _pUi->cmbDevice->setCurrentIndex(idx >= 0 ? idx : 0);
    }

    applyDevice(selectedDeviceId());
}

AddRegisterWidget::~AddRegisterWidget()
{
    delete _pUi;
}

/*!
 * \brief Build the address schema with the adapter's own deviceId field stripped out.
 *
 * The device is already fixed by the outer device selection, so the adapter's
 * addressSchema (which may declare its own generic deviceId field) must not
 * render a second, redundant device picker inside the address form.
 * \param adapterData Adapter whose data point schema to use.
 * \return The schema object ready for SchemaFormWidget, without a deviceId property.
 */
QJsonObject AddRegisterWidget::buildSchema(const AdapterData* adapterData) const
{
    QJsonObject schema = adapterData->dataPointSchema().value("addressSchema").toObject();

    QJsonObject propsObj = schema.value("properties").toObject();
    if (propsObj.contains(QStringLiteral("deviceId")))
    {
        propsObj.remove(QStringLiteral("deviceId"));
        schema["properties"] = propsObj;

        QJsonArray required = schema.value("required").toArray();
        for (int i = 0; i < required.size(); ++i)
        {
            if (required.at(i).toString() == QStringLiteral("deviceId"))
            {
                required.removeAt(i);
                schema["required"] = required;
                break;
            }
        }
    }

    return schema;
}

/*!
 * \brief Switch the widget to the given device's adapter and register schema.
 *
 * Rebuilds the address form from the device's adapter's data point schema and
 * updates the add button state based on manager availability. Also resizes
 * the enclosing popup menu, if any, to fit the rebuilt form.
 * \param deviceId Identifier of the device to use.
 */
void AddRegisterWidget::applyDevice(deviceId_t deviceId)
{
    if (_pUi->cmbDevice->count() == 0)
    {
        _pAdapterManager = nullptr;
        _pUi->lblProtocol->clear();
        _pUi->btnAdd->setEnabled(false);
        return;
    }

    const QString adapterId = _pSettingsModel->adapterIdForDevice(deviceId);
    _pAdapterManager = _pAdapterHub->adapterManager(adapterId);

    if (_pAdapterManager == nullptr)
    {
        /* Adapter isn't currently discovered/running: show the bare id rather than
         * looking it up (adapterData() would otherwise insert a phantom entry for
         * an id that was never actually registered), and clear the stale form from
         * whichever device was selected before. */
        _pUi->lblProtocol->setText(tr("Protocol: %1").arg(adapterId));
        _addressSchema = QJsonObject();
        _dataPointDefaults = QJsonObject();
        rebuildAddressForm();
        _pUi->btnAdd->setEnabled(false);
        resizeContainingMenu();
        return;
    }

    const AdapterData* adapterData = _pSettingsModel->adapterData(adapterId);
    QString protocolLabel = adapterData->name();
    if (protocolLabel.isEmpty())
    {
        protocolLabel = adapterId;
    }
    _pUi->lblProtocol->setText(tr("Protocol: %1").arg(protocolLabel));

    _addressSchema = buildSchema(adapterData);
    _dataPointDefaults = adapterData->dataPointSchema().value("defaults").toObject();
    rebuildAddressForm();

    _pUi->btnAdd->setEnabled(isSelectionUsable());

    resizeContainingMenu();
}

/*!
 * \brief Rebuilds the address form widgets from the currently cached schema and defaults.
 */
void AddRegisterWidget::rebuildAddressForm()
{
    _pAddressForm->setSchema(_addressSchema, _dataPointDefaults);
}

/*!
 * \brief Resizes the enclosing popup menu to fit this widget's current content.
 *
 * When hosted as a QWidgetAction's default widget inside a QToolButton's popup menu (as
 * RegisterDialog does for btnAdd), QMenu caches its action layout and only recomputes it for
 * specific events (its action list changing, being shown, or being resized) - it has no way to
 * know an already-embedded widget's own content changed. Without this, switching devices while
 * the popup is open leaves it clipped to whichever device's form was shown first. A synthetic resize
 * event forces QMenu to mark its cached layout dirty and recompute it, so the sizeHint() below
 * reflects the rebuilt form instead of stale, cached action rects.
 */
void AddRegisterWidget::resizeContainingMenu()
{
    auto* menu = qobject_cast<QMenu*>(window());
    if (menu == nullptr)
    {
        return;
    }

    QResizeEvent resizeEvent(menu->size(), menu->size());
    QCoreApplication::sendEvent(menu, &resizeEvent);

    menu->resize(menu->sizeHint());
}

/*!
 * \brief Closes the enclosing popup menu, if any, after a register was successfully added.
 *
 * When hosted as a QWidgetAction's default widget inside a QToolButton's popup menu (as
 * RegisterDialog does for btnAdd), clicking this widget's own "Add" button doesn't trigger
 * the QWidgetAction itself, so QMenu never closes on its own. Close it explicitly so adding
 * a register behaves like any other menu action.
 */
void AddRegisterWidget::closeContainingMenu()
{
    auto* menu = qobject_cast<QMenu*>(window());
    if (menu == nullptr)
    {
        return;
    }

    menu->close();
}

/*!
 * \brief Returns whether the currently selected device's adapter has a live manager.
 */
bool AddRegisterWidget::isSelectionUsable() const
{
    return _pAdapterManager != nullptr;
}

/*!
 * \brief Returns the ID of the device currently selected in the device combo box.
 */
deviceId_t AddRegisterWidget::selectedDeviceId() const
{
    return static_cast<deviceId_t>(_pUi->cmbDevice->currentData().toUInt());
}

/*!
 * \brief Rebuilds the address form when another device is selected.
 *
 * The curve name and axis selection are kept so switching devices only
 * replaces the address fields.
 * \param index Index of the newly selected combo box entry (unused).
 */
void AddRegisterWidget::onDeviceSelectionChanged(int index)
{
    Q_UNUSED(index);
    applyDevice(selectedDeviceId());
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
    const deviceId_t deviceId = selectedDeviceId();

    _pUi->btnAdd->setEnabled(false);
    connect(_pAdapterManager, &AdapterManager::buildExpressionResult, this, &AddRegisterWidget::onBuildExpressionResult,
            Qt::SingleShotConnection);
    _pAdapterManager->buildExpression(allValues, dataType, deviceId);
}

void AddRegisterWidget::onBuildExpressionResult(const QString& expression)
{
    /* Recompute instead of unconditionally enabling: the user may have switched
     * to a device whose adapter is unavailable while the request was in flight */
    _pUi->btnAdd->setEnabled(isSelectionUsable());

    if (expression.isEmpty())
    {
        return;
    }

    _pendingGraphData.setExpression(expression);
    emit graphDataConfigured(_pendingGraphData);

    resetFields();
    rebuildAddressForm();
    closeContainingMenu();
}

void AddRegisterWidget::resetFields()
{
    _pUi->lineName->setText("Name of curve");
    _pUi->radioPrimary->setChecked(true);
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
