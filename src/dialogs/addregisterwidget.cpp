#include "addregisterwidget.h"
#include "ui_addregisterwidget.h"

#include "ProtocolAdapter/datapointservice.h"
#include "customwidgets/schemaformwidget.h"
#include "models/device.h"
#include "models/settingsmodel.h"
#include "util/scopelogging.h"

#include <QCoreApplication>
#include <QMenu>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QVBoxLayout>

/*!
 * \brief Constructs the widget and populates it from the selected device's register schema.
 *
 * The device selector is filled with all configured devices.
 * \param pSettingsModel    Pointer to the application settings model.
 * \param pDataPointService Service that sends data point requests to the device's adapter.
 * \param parent            Optional parent widget.
 */
AddRegisterWidget::AddRegisterWidget(SettingsModel* pSettingsModel,
                                     DataPointService* pDataPointService,
                                     QWidget* parent)
    : QWidget(parent),
      _pUi(new Ui::AddRegisterWidget),
      _pAddressForm(new SchemaFormWidget(this)),
      _pSettingsModel(pSettingsModel),
      _pDataPointService(pDataPointService)
{
    _pUi->setupUi(this);

    _pUi->lineName->setFocus();

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
    connect(_pDataPointService, &DataPointService::expressionBuilt, this, &AddRegisterWidget::onExpressionBuilt);
    connect(_pDataPointService, &DataPointService::requestFailed, this, &AddRegisterWidget::onBuildRequestFailed);

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
        _pUi->lblProtocol->clear();
        _pUi->btnAdd->setEnabled(false);
        return;
    }

    const QString adapterId = _pDataPointService->adapterIdForDevice(deviceId);
    _pUi->lblProtocol->setText(tr("Protocol: %1").arg(_pSettingsModel->adapterDisplayName(adapterId)));

    /* Also clears the stale form of the previous device when this adapter has no schema */
    _addressSchema = _pDataPointService->addressSchema(deviceId);
    _dataPointDefaults = _pDataPointService->defaults(deviceId);
    rebuildAddressForm();

    /* Stay disabled while a build request is pending: a second click would replace it */
    _pUi->btnAdd->setEnabled(_pendingBuildTicket == 0 && isSelectionUsable());

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
 * SignalsDialog does for btnAdd), QMenu caches its action layout and only recomputes it for
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
 * SignalsDialog does for btnAdd), clicking this widget's own "Add" button doesn't trigger
 * the QWidgetAction itself, so QMenu never closes on its own. Close it explicitly so adding
 * a register behaves like any other menu action.
 *
 * Only called from onExpressionBuilt(), which runs on a later event loop turn than the
 * "Add" button click that triggered the request (buildExpression() replies asynchronously over
 * the adapter subprocess IPC channel) - closing the menu here never nests inside btnAdd's own
 * click handling.
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
 * \brief Returns whether the currently selected device's adapter can build expressions now.
 */
bool AddRegisterWidget::isSelectionUsable() const
{
    return _pDataPointService->isDeviceUsable(selectedDeviceId());
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
 * The signal name and axis selection are kept so switching devices only
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
    if (!isSelectionUsable())
    {
        return;
    }

    collectPendingGraphData();

    _pUi->btnAdd->setEnabled(false);
    _pendingBuildTicket = _pDataPointService->buildExpression(selectedDeviceId(), _pAddressForm->values());
}

/*!
 * \brief Emit the signal configured with the expression the adapter built for the pending request.
 * \param ticket     Ticket of the finished request; results of other requests are ignored.
 * \param expression The built data point reference.
 */
void AddRegisterWidget::onExpressionBuilt(quint64 ticket, const QString& expression)
{
    if (ticket != _pendingBuildTicket)
    {
        return;
    }
    _pendingBuildTicket = 0;

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

/*!
 * \brief Keep the form open and allow another attempt when the pending build request failed.
 * \param ticket Ticket of the failed request; failures of other requests are ignored.
 * \param error  Why the request failed.
 */
void AddRegisterWidget::onBuildRequestFailed(quint64 ticket, const QString& error)
{
    if (ticket != _pendingBuildTicket)
    {
        return;
    }
    _pendingBuildTicket = 0;

    qCWarning(scopeComm) << "AddRegisterWidget: building the expression failed:" << error;
    _pUi->btnAdd->setEnabled(isSelectionUsable());
}

void AddRegisterWidget::resetFields()
{
    _pUi->lineName->clear();
    _pUi->lineName->setFocus();
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
