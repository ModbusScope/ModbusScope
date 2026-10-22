
#include "signalsdialog.h"

#include "ProtocolAdapter/adapterhub.h"
#include "ProtocolAdapter/adaptermanager.h"
#include "ProtocolAdapter/datapointservice.h"
#include "customwidgets/actionbuttondelegate.h"
#include "datahandling/datapointusage.h"
#include "dialogs/addregisterwidget.h"
#include "dialogs/expressionsdialog.h"
#include "dialogs/ui_signalsdialog.h"
#include "models/datapointdescriptioncache.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"
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

    _pGraphDataModel = pGraphDataModel;
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
    _pUi->signalView->setModel(_pGraphDataModel);
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

    // Handle delete
    QShortcut* shortcut = new QShortcut(QKeySequence(QKeySequence::Delete), _pUi->signalView);
    connect(shortcut, &QShortcut::activated, this, &SignalsDialog::removeSelectedSignals);

    // Setup handler for buttons
    connect(_pUi->btnAdd, &QPushButton::released, this, &SignalsDialog::addDefaultSignal);
    connect(_pUi->btnRemove, &QPushButton::released, this, &SignalsDialog::removeSelectedSignals);
    connect(_pGraphDataModel, &GraphDataModel::rowsInserted, this, &SignalsDialog::onSignalInserted);

    if (!deviceIds.isEmpty())
    {
        auto pAddSignalWidget = new AddRegisterWidget(_pSettingsModel, _pDataPointService, this);
        connect(pAddSignalWidget, &AddRegisterWidget::graphDataConfigured, this, &SignalsDialog::addSignal);

        _addSignalAction = std::make_unique<QWidgetAction>(this);
        _addSignalAction->setDefaultWidget(pAddSignalWidget);
        _pUi->btnAdd->addAction(_addSignalAction.get());
    }

    setupDataPointLimitIndication();
}

SignalsDialog::~SignalsDialog()
{
    delete _pUi;
}

void SignalsDialog::addSignal(const GraphData& graphData)
{
    _pGraphDataModel->add(graphData);
}

void SignalsDialog::addDefaultSignal()
{
    _pGraphDataModel->add();
}

void SignalsDialog::activatedCell(QModelIndex modelIndex)
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

void SignalsDialog::onSignalInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(last);

    /* select the first new row, this will also make the row visible */
    _pUi->signalView->selectRow(first);
}

void SignalsDialog::removeSelectedSignals()
{
    // get list of selected rows
    QItemSelectionModel* selected = _pUi->signalView->selectionModel();
    QModelIndexList rowList = selected->selectedRows();

    // sort QModelIndexList
    // We need to remove the highest rows first
    std::sort(rowList.begin(), rowList.end(), &SignalsDialog::sortRegistersLastFirst);

    for (const QModelIndex& rowIndex : std::as_const(rowList))
    {
        _pGraphDataModel->removeRow(rowIndex.row());
    }

    if (rowList.size() > 0)
    {
        _pUi->signalView->selectRow(selectedRowAfterDelete(rowList.first().row(), _pGraphDataModel->size()));
    }
}

void SignalsDialog::handleExpressionEdit(const QModelIndex& index)
{
    if (index.column() == GraphDataModel::column::EXPRESSION)
    {
        _pUi->signalView->closePersistentEditor(index);

        ExpressionsDialog exprDialog(_pGraphDataModel, GraphIdx(index.row()), _pAdapterHub, _pSettingsModel,
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

int SignalsDialog::selectedRowAfterDelete(int deletedStartIndex, int rowCnt)
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

bool SignalsDialog::sortRegistersLastFirst(const QModelIndex& s1, const QModelIndex& s2)
{
    return s1.row() > s2.row();
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
        _pGraphDataModel->setDefaultExpression(expression);
    }
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
    connect(_pGraphDataModel, &GraphDataModel::rowsInserted, this, &SignalsDialog::updateDataPointLimitIndication);
    connect(_pGraphDataModel, &GraphDataModel::rowsRemoved, this, &SignalsDialog::updateDataPointLimitIndication);
    connect(_pGraphDataModel, &GraphDataModel::modelReset, this, &SignalsDialog::updateDataPointLimitIndication);
    connect(_pGraphDataModel, &GraphDataModel::dataChanged, this, &SignalsDialog::updateDataPointLimitIndication);

    updateDataPointLimitIndication();
}

/*!
 * \brief Build a warning message listing adapters that are asked to read more data points than
 *  they currently allow, or an empty string when every adapter is within its limit.
 */
QString SignalsDialog::dataPointLimitWarningMessage() const
{
    const QList<DataPoint> dataPoints = DataPointUsage::activeDataPoints(_pGraphDataModel);
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
