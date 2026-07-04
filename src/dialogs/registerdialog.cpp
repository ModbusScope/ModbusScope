
#include "registerdialog.h"

#include "ProtocolAdapter/adapterhub.h"
#include "ProtocolAdapter/adaptermanager.h"
#include "customwidgets/actionbuttondelegate.h"
#include "dialogs/addregisterwidget.h"
#include "dialogs/expressionsdialog.h"
#include "dialogs/ui_registerdialog.h"
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
    _pDefaultExpressionManager = defaultExpressionManager();

    if (_pDefaultExpressionManager != nullptr)
    {
        connect(_pDefaultExpressionManager, &AdapterManager::sessionStarted, this,
                &RegisterDialog::requestDefaultExpression);
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

    if (!_pAdapterHub->adapterIds().isEmpty())
    {
        auto registerPopupMenu = new AddRegisterWidget(_pSettingsModel, _pAdapterHub, this);
        connect(registerPopupMenu, &AddRegisterWidget::graphDataConfigured, this, &RegisterDialog::addRegister);

        _registerPopupAction = std::make_unique<QWidgetAction>(this);
        _registerPopupAction->setDefaultWidget(registerPopupMenu);
        _pUi->btnAdd->addAction(_registerPopupAction.get());
    }
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
 * \brief Determine which adapter manager drives the default new-register expression.
 *
 * The modbus adapter is preferred to keep the existing single-adapter behavior;
 * when it is not available the first discovered adapter is used.
 * \return Pointer to the manager, or nullptr when no adapters are available.
 */
AdapterManager* RegisterDialog::defaultExpressionManager() const
{
    AdapterManager* pManager = _pAdapterHub->adapterManager(cModbusAdapterId);
    if (pManager == nullptr)
    {
        const QStringList adapterIds = _pAdapterHub->adapterIds();
        if (!adapterIds.isEmpty())
        {
            pManager = _pAdapterHub->adapterManager(adapterIds.first());
        }
    }

    return pManager;
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
    _pDefaultExpressionManager->buildExpression(addressFields, dataType, 0);
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
