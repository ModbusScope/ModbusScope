
#include "signalsdialog.h"

#include "ProtocolAdapter/adapterhub.h"
#include "ProtocolAdapter/datapointservice.h"
#include "customwidgets/actionbuttondelegate.h"
#include "datahandling/datapointusage.h"
#include "dialogs/expressionsdialog.h"
#include "dialogs/signaleditorpanel.h"
#include "dialogs/signaltableproxymodel.h"
#include "dialogs/ui_signalsdialog.h"
#include "models/datapointdescriptioncache.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"
#include "models/signallistdraft.h"
#include "valueaxisdelegate.h"

#include <QColorDialog>
#include <QMessageBox>
#include <QShortcut>

namespace {

/*!
 * \brief Returns the row to select after removing signals.
 * \param firstRemovedRow Lowest row that was removed.
 * \param rowCount        Number of rows left after the removal.
 * \return The row that took the place of the first removed row, the last row when the removed
 *         rows were at the end, or -1 when no rows are left.
 */
int rowAfterRemoval(int firstRemovedRow, int rowCount)
{
    if (rowCount <= 0)
    {
        return -1;
    }

    if (firstRemovedRow < rowCount)
    {
        return firstRemovedRow;
    }

    return rowCount - 1;
}

} // namespace

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
    _pTableModel = new SignalTableProxyModel(_pDraft->model(), _pDescriptionCache, this);

    // Setup signalView
    _pUi->signalView->setModel(_pTableModel);
    _pUi->signalView->verticalHeader()->hide();

    /* The view owns its delegates, so they live as long as the view asks them for sizes */
    _pValueAxisDelegate = new ValueAxisDelegate(_pUi->signalView);
    _pUi->signalView->setItemDelegateForColumn(GraphDataModel::column::VALUE_AXIS, _pValueAxisDelegate);

    _pExpressionDelegate = new ActionButtonDelegate(_pUi->signalView);
    _pExpressionDelegate->setCharacter(QChar(0x2026));
    connect(_pExpressionDelegate, &ActionButtonDelegate::clicked, this, &SignalsDialog::handleExpressionEdit);
    _pUi->signalView->setItemDelegateForColumn(GraphDataModel::column::EXPRESSION, _pExpressionDelegate);

    /* Don't stretch columns */
    _pUi->signalView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    /* Except following columns */
    _pUi->signalView->horizontalHeader()->setSectionResizeMode(GraphDataModel::column::TEXT, QHeaderView::Stretch);
    _pUi->signalView->horizontalHeader()->setSectionResizeMode(GraphDataModel::column::EXPRESSION,
                                                               QHeaderView::Stretch);
    _pUi->signalView->horizontalHeader()->setSectionResizeMode(_pTableModel->dataPointColumn(), QHeaderView::Stretch);

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
 * \brief Closes the dialog without applying the changes.
 *
 * Cancel, Esc and closing the window all end here. When the signal list was changed, the user
 * confirms first that the changes are discarded.
 */
void SignalsDialog::reject()
{
    bool bDiscard = true;
    if (_pDraft->hasChanges())
    {
        const auto reply = QMessageBox::question(this, QStringLiteral("Discard changes"),
                                                 QStringLiteral("Discard the changes to the signals?"),
                                                 QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel);
        bDiscard = reply == QMessageBox::Discard;
    }

    if (bDiscard)
    {
        QDialog::reject();
    }
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
            _pTableModel->setData(modelIndex, color, Qt::EditRole);
        }
    }
}

void SignalsDialog::onSignalInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(last);

    /* Only scroll: selecting a row is reserved for editing that signal */
    _pUi->signalView->scrollTo(_pTableModel->index(first, 0));
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

    _pUi->signalView->selectRow(rowAfterRemoval(firstRow, _pDraft->model()->size()));
}

void SignalsDialog::handleExpressionEdit(const QModelIndex& index)
{
    if (index.column() == GraphDataModel::column::EXPRESSION)
    {
        _pUi->signalView->closePersistentEditor(index);

        openExpressionDialog(GraphIdx(index.row()));
    }
}

/*!
 * \brief Open Compose expression for a signal; Accept writes the expression to the draft.
 * \param graphIdx The signal to edit.
 */
void SignalsDialog::openExpressionDialog(GraphIdx graphIdx)
{
    ExpressionsDialog exprDialog(_pDraft->model(), graphIdx, _pAdapterHub, _pSettingsModel, _pDataPointService,
                                 _pDescriptionCache, this);

    exprDialog.exec();
}

/*!
 * \brief Edit the selected signal in the panel; without a selection the panel adds signals.
 */
void SignalsDialog::onSelectionChanged()
{
    const QModelIndexList rowList = _pUi->signalView->selectionModel()->selectedRows();
    if (rowList.size() == 1)
    {
        _pPanel->editSignal(GraphIdx(rowList.first().row()));
    }
    else
    {
        _pPanel->showAddMode();
    }
}

/*!
 * \brief Place the signal editor panel next to the table; the panel opens at a comfortable width
 *  and extra width goes to the table.
 *
 * Selecting a row edits that signal in the panel.
 */
void SignalsDialog::setupEditorPanel()
{
    _pPanel = new SignalEditorPanel(_pDraft->model(), _pSettingsModel, _pDataPointService, _pDescriptionCache, this);
    _pUi->panelScrollArea->setWidget(_pPanel);

    connect(_pUi->signalView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &SignalsDialog::onSelectionChanged);
    connect(_pPanel, &SignalEditorPanel::editExpressionRequested, this, &SignalsDialog::openExpressionDialog);

    _pUi->splitter->setStretchFactor(0, 1);
    _pUi->splitter->setStretchFactor(1, 0);
    _pUi->splitter->setCollapsible(0, false);
    _pUi->splitter->setCollapsible(1, false);
    _pUi->splitter->setSizes({ width() - cPanelWidth, cPanelWidth });

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

    warnings.append(QStringLiteral("A license removes this limit - see Help > About > Request License."));
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
