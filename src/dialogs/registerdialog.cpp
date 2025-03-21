
#include <QColorDialog>
#include "expressiondelegate.h"
#include "importmbcdialog.h"
#include "expressionsdialog.h"
#include "addregisterwidget.h"

#include "graphdatamodel.h"
#include "guimodel.h"
#include "settingsmodel.h"
#include "registervalueaxisdelegate.h"

#include "registerdialog.h"
#include "ui_registerdialog.h"

RegisterDialog::RegisterDialog(GuiModel *pGuiModel, GraphDataModel * pGraphDataModel, SettingsModel *pSettingsModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::RegisterDialog)
{
    _pUi->setupUi(this);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    _pGraphDataModel = pGraphDataModel;
    _pGuiModel = pGuiModel;
    _pSettingsModel = pSettingsModel;

    // Setup registerView
    _pUi->registerView->setModel(_pGraphDataModel);
    _pUi->registerView->verticalHeader()->hide();

    _valueAxisDelegate = std::make_unique<RegisterValueAxisDelegate>(_pGraphDataModel, _pUi->registerView);
    _pUi->registerView->setItemDelegateForColumn(GraphDataModel::column::VALUE_AXIS, _valueAxisDelegate.get());

    _expressionDelegate = std::make_unique<ExpressionDelegate>(_pGraphDataModel, _pUi->registerView);
    connect(_expressionDelegate.get(), &ExpressionDelegate::clicked, this, &RegisterDialog::handleExpressionEdit);
    _pUi->registerView->setItemDelegateForColumn(GraphDataModel::column::EXPRESSION, _expressionDelegate.get());

    /* Don't stretch columns */
    _pUi->registerView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    /* Except following columns */
    _pUi->registerView->horizontalHeader()->setSectionResizeMode(GraphDataModel::column::TEXT, QHeaderView::Stretch);
    _pUi->registerView->horizontalHeader()->setSectionResizeMode(GraphDataModel::column::EXPRESSION, QHeaderView::Stretch);

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

    // Handle delete
    QShortcut* shortcut = new QShortcut(QKeySequence(QKeySequence::Delete), _pUi->registerView);
    connect(shortcut, &QShortcut::activated, this, &RegisterDialog::removeRegisterRow);

    // Setup handler for buttons
    connect(_pUi->btnAdd, &QPushButton::released, this, &RegisterDialog::addDefaultRegister);
    connect(_pUi->btnRemove, &QPushButton::released, this, &RegisterDialog::removeRegisterRow);
    connect(_pGraphDataModel, &GraphDataModel::rowsInserted, this, &RegisterDialog::onRegisterInserted);

    auto registerPopupMenu = new AddRegisterWidget(_pSettingsModel, this);
    connect(registerPopupMenu, &AddRegisterWidget::graphDataConfigured, this, &RegisterDialog::addRegister);

    _registerPopupAction = std::make_unique<QWidgetAction>(this);
    _registerPopupAction->setDefaultWidget(registerPopupMenu);
    _pUi->btnAdd->addAction(_registerPopupAction.get());
}

RegisterDialog::~RegisterDialog()
{
    delete _pUi;
}

void RegisterDialog::addRegister(const GraphData &graphData)
{
    _pGraphDataModel->add(graphData);
}

void RegisterDialog::addDefaultRegister()
{
    _pGraphDataModel->add();
}

void RegisterDialog::activatedCell(QModelIndex modelIndex)
{
    if (
        (modelIndex.column() == GraphDataModel::column::COLOR)
        && (modelIndex.row() < _pGraphDataModel->size())
        )
    {
        QColor color = QColorDialog::getColor(_pGraphDataModel->color(modelIndex.row()));

        if (color.isValid())
        {
            _pGraphDataModel->setData(modelIndex, color, Qt::EditRole);
        }
    }
}

void RegisterDialog::onRegisterInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(last);

    /* select the first new row, this will also make the row visible */
    _pUi->registerView->selectRow(first);
}

void RegisterDialog::removeRegisterRow()
{
    // get list of selected rows
    QItemSelectionModel *selected = _pUi->registerView->selectionModel();
    QModelIndexList rowList = selected->selectedRows();

    // sort QModelIndexList
    // We need to remove the highest rows first
    std::sort(rowList.begin(), rowList.end(), &RegisterDialog::sortRegistersLastFirst);

    foreach(QModelIndex rowIndex, rowList)
    {
        _pGraphDataModel->removeRow(rowIndex.row());
    }

    if (rowList.size() > 0)
    {
        _pUi->registerView->selectRow(selectedRowAfterDelete(rowList.first().row(), _pGraphDataModel->size()));
    }
}

void RegisterDialog::handleExpressionEdit(int row)
{
    QModelIndex idx = _pGraphDataModel->index(row, GraphDataModel::column::EXPRESSION, QModelIndex());
    _pUi->registerView->closePersistentEditor(idx);

    ExpressionsDialog exprDialog(_pGraphDataModel, row, this);

    exprDialog.exec();
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

bool RegisterDialog::sortRegistersLastFirst(const QModelIndex &s1, const QModelIndex &s2)
{
    return s1.row() > s2.row();
}
