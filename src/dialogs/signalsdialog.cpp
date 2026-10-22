
#include "signalsdialog.h"

#include "ProtocolAdapter/adapterhub.h"
#include "ProtocolAdapter/adaptermanager.h"
#include "ProtocolAdapter/datapointservice.h"
#include "customwidgets/actionbuttondelegate.h"
#include "datahandling/datapointusage.h"
#include "dialogs/expressionsdialog.h"
#include "dialogs/signaleditorpanel.h"
#include "dialogs/ui_signalsdialog.h"
#include "models/datapointdescriptioncache.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"
#include "models/signallistdraft.h"
#include "valueaxisdelegate.h"

#include <QColorDialog>
#include <QShortcut>

SignalsDialog::SignalsDialog(GraphDataModel* pGraphDataModel,
                             SettingsModel* pSettingsModel,
                             AdapterHub* pAdapterHub,
                             QWidget* parent)
    : QDialog(parent), _pUi(new Ui::SignalsDialog)
{
    _pUi->setupUi(this);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    _pDraft = new SignalListDraft(pGraphDataModel, pSettingsModel, this);
    _pSettingsModel = pSettingsModel;
    _pAdapterHub = pAdapterHub;
    _pDataPointService = new DataPointService(_pSettingsModel, _pAdapterHub, this);
    _pDescriptionCache = new DataPointDescriptionCache(_pDataPointService, this);
    connect(_pDataPointService, &DataPointService::expressionBuilt, this, &SignalsDialog::onDefaultExpressionBuilt);

    const QList<deviceId_t> deviceIds = _pSettingsModel->deviceList();
    initDefaultExpressionTarget(deviceIds);

    if (_pDefaultExpressionManager != nullptr)
    {
        connect(_pDefaultExpressionManager, &AdapterManager::sessionStarted, this,
                &SignalsDialog::requestDefaultExpression);

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

    // Setup signalView
    _pUi->signalView->setModel(_pDraft->model());
    _pUi->signalView->verticalHeader()->hide();

    _valueAxisDelegate = std::make_unique<ValueAxisDelegate>(_pUi->signalView);
    _pUi->signalView->setItemDelegateForColumn(GraphDataModel::column::VALUE_AXIS, _valueAxisDelegate.get());

    _expressionDelegate = std::make_unique<ActionButtonDelegate>(_pUi->signalView);
    _expressionDelegate->setCharacter(QChar(0x2026));
    connect(_expressionDelegate.get(), &ActionButtonDelegate::clicked, this, &SignalsDialog::handleExpressionEdit);
    _pUi->signalView->setItemDelegateForColumn(GraphDataModel::column::EXPRESSION, _expressionDelegate.get());

    /* Don't stretch columns */
    _pUi->signalView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    /* Except following columns */
    _pUi->signalView->horizontalHeader()->setSectionResizeMode(GraphDataModel::column::TEXT, QHeaderView::Stretch);
    _pUi->signalView->horizontalHeader()->setSectionResizeMode(GraphDataModel::column::EXPRESSION,
                                                               QHeaderView::Stretch);

    auto triggers = QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed;
    _pUi->signalView->setEditTriggers(triggers);
    _pUi->signalView->setSelectionBehavior(QAbstractItemView::SelectRows);
    _pUi->signalView->setSelectionMode(QAbstractItemView::SingleSelection);
    _pUi->signalView->setDragEnabled(true);
    _pUi->signalView->setAcceptDrops(false);
    _pUi->signalView->setDropIndicatorShown(true);
    _pUi->signalView->setDefaultDropAction(Qt::MoveAction);
    _pUi->signalView->setDragDropMode(QTableView::InternalMove);
    _pUi->signalView->setDragDropOverwriteMode(false);

    // Handle cell active signal
    connect(_pUi->signalView, &QTableView::activated, this, &SignalsDialog::activatedCell);

    _pUi->signalView->setStyle(&_centeredBoxStyle);

    /* Only while the table has the focus, so Delete in a panel field edits that field */
    QShortcut* shortcut = new QShortcut(QKeySequence(QKeySequence::Delete), _pUi->signalView);
    shortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(shortcut, &QShortcut::activated, this, &SignalsDialog::removeSelectedSignals);

    // Setup handler for buttons
    connect(_pUi->btnAdd, &QPushButton::clicked, this, &SignalsDialog::startAddSignal);
    connect(_pUi->btnRemove, &QPushButton::clicked, this, &SignalsDialog::removeSelectedSignals);
    connect(_pDraft->model(), &GraphDataModel::rowsInserted, this, &SignalsDialog::onSignalInserted);

    setupEditorPanel();
    setupDataPointLimitIndication();
}

SignalsDialog::~SignalsDialog()
{
    delete _pUi;
}

/*!
 * \brief Applies the changes made in the dialog to the signal list and closes the dialog.
 */
void SignalsDialog::accept()
{
    /* OK through its shortcut keeps the focus in an open cell editor; moving it commits the edit */
    _pUi->buttonBox->setFocus();

    _pDraft->apply();
    QDialog::accept();
}

/*!
 * \brief Put the panel in add mode: no row selected, the name field ready for input.
 */
void SignalsDialog::startAddSignal()
{
    _pUi->signalView->clearSelection();
    _pPanel->startAdd();
}

void SignalsDialog::activatedCell(QModelIndex modelIndex)
{
    if ((modelIndex.column() == GraphDataModel::column::COLOR) && (modelIndex.row() < _pDraft->model()->size()))
    {
        QColor color = QColorDialog::getColor(_pDraft->model()->color(GraphIdx(modelIndex.row())));

        if (color.isValid())
        {
            _pDraft->model()->setData(modelIndex, color, Qt::EditRole);
        }
    }
}

void SignalsDialog::onSignalInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(last);

    /* Only scroll: selecting a row is reserved for editing that signal */
    _pUi->signalView->scrollTo(_pDraft->model()->index(first, 0));
}

void SignalsDialog::removeSelectedSignals()
{
    const QModelIndexList rowList = _pUi->signalView->selectionModel()->selectedRows();
    if (rowList.isEmpty())
    {
        return;
    }

    QList<GraphIdx> indices;
    int firstRow = rowList.first().row();
    for (const QModelIndex& rowIndex : rowList)
    {
        indices.append(GraphIdx(rowIndex.row()));
        firstRow = qMin(firstRow, rowIndex.row());
    }

    _pDraft->model()->removeSignals(indices);

    _pUi->signalView->selectRow(GraphDataModel::rowAfterRemoval(firstRow, _pDraft->model()->size()));
}

void SignalsDialog::handleExpressionEdit(const QModelIndex& index)
{
    if (index.column() == GraphDataModel::column::EXPRESSION)
    {
        _pUi->signalView->closePersistentEditor(index);

        ExpressionsDialog exprDialog(_pDraft->model(), GraphIdx(index.row()), _pAdapterHub, _pSettingsModel,
                                     _pDescriptionCache, this);

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
void SignalsDialog::initDefaultExpressionTarget(const QList<deviceId_t>& deviceIds)
{
    if (deviceIds.isEmpty())
    {
        _pDefaultExpressionManager = nullptr;
        return;
    }

    _defaultExpressionDeviceId = deviceIds.first();
    const QString adapterId = _pDataPointService->adapterIdForDevice(_defaultExpressionDeviceId);
    _pDefaultExpressionManager = _pAdapterHub->adapterManager(adapterId);
}

/*!
 * \brief Requests a default expression from the adapter using its own schema defaults.
 *
 * Called when the adapter session starts. The result is used to keep the default new-register
 * expression in sync with whatever the adapter considers its default data point.
 */
void SignalsDialog::requestDefaultExpression()
{
    if (_pDefaultExpressionManager == nullptr)
    {
        return;
    }

    const QJsonObject defaults = _pDataPointService->defaults(_defaultExpressionDeviceId);
    if (defaults.isEmpty())
    {
        return;
    }

    /* Only the latest request counts: a reply to an earlier one is ignored */
    _defaultExpressionTicket = _pDataPointService->buildExpression(_defaultExpressionDeviceId, defaults);
}

/*!
 * \brief Applies the adapter-provided default expression to the graph data model.
 * \param ticket     Ticket of the finished request; results of other requests are ignored.
 * \param expression The default expression string returned by the adapter.
 */
void SignalsDialog::onDefaultExpressionBuilt(quint64 ticket, const QString& expression)
{
    if (ticket != _defaultExpressionTicket)
    {
        return;
    }

    if (!expression.isEmpty())
    {
        _pDraft->model()->setDefaultExpression(expression);
    }
}

/*!
 * \brief Place the signal editor panel next to the table; extra width goes to the table.
 */
void SignalsDialog::setupEditorPanel()
{
    _pPanel = new SignalEditorPanel(_pDraft->model(), _pSettingsModel, _pDataPointService, this);
    _pUi->panelScrollArea->setWidget(_pPanel);

    _pUi->splitter->setStretchFactor(0, 1);
    _pUi->splitter->setStretchFactor(1, 0);
    _pUi->splitter->setCollapsible(0, false);
    _pUi->splitter->setCollapsible(1, false);

    setMinimumSize(cMinimumWidth, cMinimumHeight);
}

/*!
 * \brief Style the data point limit warning and keep it in sync with the signal list.
 */
void SignalsDialog::setupDataPointLimitIndication()
{
    /* Adapter names come from the adapter subprocess's describe response, so treat them as
     * untrusted: force plain text to prevent a misbehaving adapter from injecting rich-text
     * formatting into this label. */
    _pUi->dataPointLimitWarningLabel->setTextFormat(Qt::PlainText);
    _pUi->dataPointLimitWarningLabel->setStyleSheet("QLabel { color: #b35900; }");

    /* dataChanged covers activating a signal and editing its expression, including edits made
     * through the expression dialog: the model emits it for the complete row. */
    connect(_pDraft->model(), &GraphDataModel::rowsInserted, this, &SignalsDialog::updateDataPointLimitIndication);
    connect(_pDraft->model(), &GraphDataModel::rowsRemoved, this, &SignalsDialog::updateDataPointLimitIndication);
    connect(_pDraft->model(), &GraphDataModel::modelReset, this, &SignalsDialog::updateDataPointLimitIndication);
    connect(_pDraft->model(), &GraphDataModel::dataChanged, this, &SignalsDialog::updateDataPointLimitIndication);

    updateDataPointLimitIndication();
}

/*!
 * \brief Build a warning message listing adapters that are asked to read more data points than
 *  they currently allow, or an empty string when every adapter is within its limit.
 */
QString SignalsDialog::dataPointLimitWarningMessage() const
{
    const QList<DataPoint> dataPoints = DataPointUsage::activeDataPoints(_pDraft->model());
    QStringList warnings = DataPointUsage::limitWarnings(dataPoints, _pSettingsModel);
    if (warnings.isEmpty())
    {
        return QString();
    }

    warnings.append(tr("A license removes this limit - see Help > About > Request License."));
    return warnings.join('\n');
}

/*!
 * \brief Show the data point limit warning while a limit is exceeded, hide it otherwise.
 *
 * Logging is never blocked: the adapter reports the limit itself when a session starts.
 */
void SignalsDialog::updateDataPointLimitIndication()
{
    const QString message = dataPointLimitWarningMessage();
    _pUi->dataPointLimitWarningLabel->setText(message);
    _pUi->dataPointLimitWarningLabel->setVisible(!message.isEmpty());
}
