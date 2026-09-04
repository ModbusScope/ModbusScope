
#include "registerdialog.h"

#include "ProtocolAdapter/adapterhub.h"
#include "ProtocolAdapter/adaptermanager.h"
#include "customwidgets/actionbuttondelegate.h"
#include "datahandling/datapointusage.h"
#include "dialogs/addregisterwidget.h"
#include "dialogs/expressionsdialog.h"
#include "dialogs/ui_registerdialog.h"
#include "models/adapterdata.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"
#include "registervalueaxisdelegate.h"

#include <QColorDialog>
#include <QShortcut>

RegisterDialog::RegisterDialog(GraphDataModel* pGraphDataModel,
                               SettingsModel* pSettingsModel,
                               AdapterHub* pAdapterHub,
                               QWidget* parent)
    : QDialog(parent), _pUi(new Ui::RegisterDialog)
{
    _pUi->setupUi(this);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    _pGraphDataModel = pGraphDataModel;
    _pSettingsModel = pSettingsModel;
    _pAdapterHub = pAdapterHub;

    const QList<deviceId_t> deviceIds = _pSettingsModel->deviceList();
    initDefaultExpressionTarget(deviceIds);

    if (_pDefaultExpressionManager != nullptr)
    {
        connect(_pDefaultExpressionManager, &AdapterManager::sessionStarted, this,
                &RegisterDialog::requestDefaultExpression);

        /* sessionStarted() only fires once per session: if the adapter is already active, or
         * already past adapter.describe and awaiting configuration (both states buildExpression()
         * accepts, and the schema is already available in either), that emission has already
         * happened and the connection above will never fire for it. Request the default
         * expression directly for that case. */
        if (_pDefaultExpressionManager->isAdapterActive() || _pDefaultExpressionManager->isAdapterReady())
        {
            requestDefaultExpression();
        }
    }

    // Setup registerView
    _pUi->registerView->setModel(_pGraphDataModel);
    _pUi->registerView->verticalHeader()->hide();

    _valueAxisDelegate = std::make_unique<RegisterValueAxisDelegate>(_pUi->registerView);
    _pUi->registerView->setItemDelegateForColumn(GraphDataModel::column::VALUE_AXIS, _valueAxisDelegate.get());

    _expressionDelegate = std::make_unique<ActionButtonDelegate>(_pUi->registerView);
    _expressionDelegate->setCharacter(QChar(0x2026));
    connect(_expressionDelegate.get(), &ActionButtonDelegate::clicked, this, &RegisterDialog::handleExpressionEdit);
    _pUi->registerView->setItemDelegateForColumn(GraphDataModel::column::EXPRESSION, _expressionDelegate.get());

    /* Don't stretch columns */
    _pUi->registerView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    /* Except following columns */
    _pUi->registerView->horizontalHeader()->setSectionResizeMode(GraphDataModel::column::TEXT, QHeaderView::Stretch);
    _pUi->registerView->horizontalHeader()->setSectionResizeMode(GraphDataModel::column::EXPRESSION,
                                                                 QHeaderView::Stretch);

    auto triggers = QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed;
    _pUi->registerView->setEditTriggers(triggers);
    _pUi->registerView->setSelectionBehavior(QAbstractItemView::SelectRows);
    _pUi->registerView->setSelectionMode(QAbstractItemView::SingleSelection);
    _pUi->registerView->setDragEnabled(true);
    _pUi->registerView->setAcceptDrops(false);
    _pUi->registerView->setDropIndicatorShown(true);
    _pUi->registerView->setDefaultDropAction(Qt::MoveAction);
    _pUi->registerView->setDragDropMode(QTableView::InternalMove);
    _pUi->registerView->setDragDropOverwriteMode(false);

    // Handle cell active signal
    connect(_pUi->registerView, &QTableView::activated, this, &RegisterDialog::activatedCell);

    _pUi->registerView->setStyle(&_centeredBoxStyle);

    // Handle delete
    QShortcut* shortcut = new QShortcut(QKeySequence(QKeySequence::Delete), _pUi->registerView);
    connect(shortcut, &QShortcut::activated, this, &RegisterDialog::removeRegisterRow);

    // Setup handler for buttons
    connect(_pUi->btnAdd, &QPushButton::released, this, &RegisterDialog::addDefaultRegister);
    connect(_pUi->btnRemove, &QPushButton::released, this, &RegisterDialog::removeRegisterRow);
    connect(_pGraphDataModel, &GraphDataModel::rowsInserted, this, &RegisterDialog::onRegisterInserted);

    if (!deviceIds.isEmpty())
    {
        auto registerPopupMenu = new AddRegisterWidget(_pSettingsModel, _pAdapterHub, this);
        connect(registerPopupMenu, &AddRegisterWidget::graphDataConfigured, this, &RegisterDialog::addRegister);

        _registerPopupAction = std::make_unique<QWidgetAction>(this);
        _registerPopupAction->setDefaultWidget(registerPopupMenu);
        _pUi->btnAdd->addAction(_registerPopupAction.get());
    }

    setupDataPointLimitIndication();
}

RegisterDialog::~RegisterDialog()
{
    delete _pUi;
}

void RegisterDialog::addRegister(const GraphData& graphData)
{
    _pGraphDataModel->add(graphData);
}

void RegisterDialog::addDefaultRegister()
{
    _pGraphDataModel->add();
}

void RegisterDialog::activatedCell(QModelIndex modelIndex)
{
    if ((modelIndex.column() == GraphDataModel::column::COLOR) && (modelIndex.row() < _pGraphDataModel->size()))
    {
        QColor color = QColorDialog::getColor(_pGraphDataModel->color(GraphIdx(modelIndex.row())));

        if (color.isValid())
        {
            _pGraphDataModel->setData(modelIndex, color, Qt::EditRole);
        }
    }
}

void RegisterDialog::onRegisterInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(last);

    /* select the first new row, this will also make the row visible */
    _pUi->registerView->selectRow(first);
}

void RegisterDialog::removeRegisterRow()
{
    // get list of selected rows
    QItemSelectionModel* selected = _pUi->registerView->selectionModel();
    QModelIndexList rowList = selected->selectedRows();

    // sort QModelIndexList
    // We need to remove the highest rows first
    std::sort(rowList.begin(), rowList.end(), &RegisterDialog::sortRegistersLastFirst);

    for (const QModelIndex& rowIndex : std::as_const(rowList))
    {
        _pGraphDataModel->removeRow(rowIndex.row());
    }

    if (rowList.size() > 0)
    {
        _pUi->registerView->selectRow(selectedRowAfterDelete(rowList.first().row(), _pGraphDataModel->size()));
    }
}

void RegisterDialog::handleExpressionEdit(const QModelIndex& index)
{
    if (index.column() == GraphDataModel::column::EXPRESSION)
    {
        _pUi->registerView->closePersistentEditor(index);

        ExpressionsDialog exprDialog(_pGraphDataModel, GraphIdx(index.row()), _pAdapterHub, _pSettingsModel, this);

        exprDialog.exec();
    }
}

/*!
 * \brief Determine which device and adapter manager drive the default new-register expression.
 *
 * Uses the first configured device and its adapter, consistent with AddRegisterWidget's
 * device-first selection. Sets _pDefaultExpressionManager to nullptr when no devices are
 * configured.
 * \param deviceIds The currently configured device IDs, as returned by SettingsModel::deviceList().
 */
void RegisterDialog::initDefaultExpressionTarget(const QList<deviceId_t>& deviceIds)
{
    if (deviceIds.isEmpty())
    {
        _pDefaultExpressionManager = nullptr;
        return;
    }

    _defaultExpressionDeviceId = deviceIds.first();
    const QString adapterId = _pSettingsModel->adapterIdForDevice(_defaultExpressionDeviceId);
    _pDefaultExpressionManager = _pAdapterHub->adapterManager(adapterId);
}

int RegisterDialog::selectedRowAfterDelete(int deletedStartIndex, int rowCnt)
{
    int nextSelectedRow = -1;

    if (rowCnt > 0)
    {
        if (deletedStartIndex < rowCnt)
        {
            nextSelectedRow = deletedStartIndex;
        }
        else
        {

            nextSelectedRow = rowCnt - 1;
        }
    }

    return nextSelectedRow;
}

bool RegisterDialog::sortRegistersLastFirst(const QModelIndex& s1, const QModelIndex& s2)
{
    return s1.row() > s2.row();
}

/*!
 * \brief Requests a default expression from the adapter using its own schema defaults.
 *
 * Called when the adapter session starts. The result is used to keep the default new-register
 * expression in sync with whatever the adapter considers its default data point.
 */
void RegisterDialog::requestDefaultExpression()
{
    if (_pDefaultExpressionManager == nullptr)
    {
        return;
    }

    const QJsonObject defaults = _pSettingsModel->adapterData(_pDefaultExpressionManager->adapterId())
                                   ->dataPointSchema()
                                   .value("defaults")
                                   .toObject();
    if (defaults.isEmpty())
    {
        return;
    }

    /* Disconnect any pending connection from a previous session before installing a new one */
    QObject::disconnect(_pDefaultExpressionManager, &AdapterManager::buildExpressionResult, this,
                        &RegisterDialog::onDefaultExpressionBuilt);
    connect(_pDefaultExpressionManager, &AdapterManager::buildExpressionResult, this,
            &RegisterDialog::onDefaultExpressionBuilt, Qt::SingleShotConnection);

    QJsonObject addressFields = defaults;
    const QString dataType = addressFields.take(QStringLiteral("dataType")).toString();
    _pDefaultExpressionManager->buildExpression(addressFields, dataType, _defaultExpressionDeviceId);
}

/*!
 * \brief Applies the adapter-provided default expression to the graph data model.
 * \param expression The default expression string returned by the adapter.
 */
void RegisterDialog::onDefaultExpressionBuilt(const QString& expression)
{
    if (!expression.isEmpty())
    {
        _pGraphDataModel->setDefaultExpression(expression);
    }
}

/*!
 * \brief Style the data point limit warning and keep it in sync with the signal list.
 */
void RegisterDialog::setupDataPointLimitIndication()
{
    /* Adapter names come from the adapter subprocess's describe response, so treat them as
     * untrusted: force plain text to prevent a misbehaving adapter from injecting rich-text
     * formatting into this label. */
    _pUi->dataPointLimitWarningLabel->setTextFormat(Qt::PlainText);
    _pUi->dataPointLimitWarningLabel->setStyleSheet("QLabel { color: #b35900; }");

    /* dataChanged covers activating a signal and editing its expression, including edits made
     * through the expression dialog: the model emits it for the complete row. */
    connect(_pGraphDataModel, &GraphDataModel::rowsInserted, this, &RegisterDialog::updateDataPointLimitIndication);
    connect(_pGraphDataModel, &GraphDataModel::rowsRemoved, this, &RegisterDialog::updateDataPointLimitIndication);
    connect(_pGraphDataModel, &GraphDataModel::modelReset, this, &RegisterDialog::updateDataPointLimitIndication);
    connect(_pGraphDataModel, &GraphDataModel::dataChanged, this, &RegisterDialog::updateDataPointLimitIndication);

    updateDataPointLimitIndication();
}

/*!
 * \brief Build a warning message listing adapters that are asked to read more data points than
 *  they currently allow, or an empty string when every adapter is within its limit.
 *
 * The limit is the adapter's reported capabilities.maxRegisters, which is only present while the
 * adapter enforces a cap (for example when no valid license is found).
 */
QString RegisterDialog::dataPointLimitWarningMessage() const
{
    const QList<DataPoint> dataPoints = DataPointUsage::activeDataPoints(_pGraphDataModel);
    const QMap<QString, int> countByAdapter = DataPointUsage::countPerAdapter(dataPoints, _pSettingsModel);

    /* adapterData() inserts a default entry for an unknown adapterId, which would leave a phantom
       entry behind for a warning alone. An adapter SettingsModel has never heard of cannot have
       reported a limit either, so there is nothing to warn about. */
    const QStringList knownAdapterIds = _pSettingsModel->adapterIds();

    QStringList warnings;
    for (auto it = countByAdapter.constBegin(); it != countByAdapter.constEnd(); ++it)
    {
        if (!knownAdapterIds.contains(it.key()))
        {
            continue;
        }

        const AdapterData* pAdapter = _pSettingsModel->adapterData(it.key());
        const int dataPointLimit = pAdapter->maxRegisters();
        if (it.value() > dataPointLimit)
        {
            const QString adapterName = pAdapter->name().isEmpty() ? it.key() : pAdapter->name();
            warnings.append(QString("%1 allows at most %2 data point(s), but %3 are configured.")
                              .arg(adapterName)
                              .arg(dataPointLimit)
                              .arg(it.value()));
        }
    }

    return warnings.join('\n');
}

/*!
 * \brief Show the data point limit warning while a limit is exceeded, hide it otherwise.
 *
 * Logging is never blocked: the adapter reports the limit itself when a session starts.
 */
void RegisterDialog::updateDataPointLimitIndication()
{
    const QString message = dataPointLimitWarningMessage();
    _pUi->dataPointLimitWarningLabel->setText(message);
    _pUi->dataPointLimitWarningLabel->setVisible(!message.isEmpty());
}
