
#include <QColorDialog>
#include "registerdialog.h"
#include "importmbcdialog.h"
#include "expressionsdialog.h"
#include "addregisterwidget.h"

#include "graphdatamodel.h"
#include "guimodel.h"
#include "settingsmodel.h"
#include "registervalueaxisdelegate.h"

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

    auto cbValueAxis = new RegisterValueAxisDelegate(_pGraphDataModel, _pUi->registerView);
    _pUi->registerView->setItemDelegateForColumn(GraphDataModel::column::VALUE_AXIS, cbValueAxis);

    /* Don't stretch columns */
    _pUi->registerView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    /* Except following columns */
    _pUi->registerView->horizontalHeader()->setSectionResizeMode(GraphDataModel::column::TEXT, QHeaderView::Stretch);
    _pUi->registerView->horizontalHeader()->setSectionResizeMode(GraphDataModel::column::EXPRESSION, QHeaderView::Stretch);

    // Select using click, shift and control
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
    connect(_pUi->btnImportFromMbc, &QPushButton::released, this, &RegisterDialog::showImportDialog);
    connect(_pUi->btnAdd, &QPushButton::released, this, &RegisterDialog::addDefaultRegister);
    connect(_pUi->btnRemove, &QPushButton::released, this, &RegisterDialog::removeRegisterRow);
    connect(_pGraphDataModel, &GraphDataModel::rowsInserted, this, &RegisterDialog::onRegisterInserted);

    auto registerPopup = new AddRegisterWidget(_pSettingsModel, this);
    connect(registerPopup, &AddRegisterWidget::graphDataConfigured, this, &RegisterDialog::addRegister);
    auto popupAction = new QWidgetAction(this);
    popupAction->setDefaultWidget(registerPopup);
    _pUi->btnAdd->addAction(popupAction);
}

RegisterDialog::~RegisterDialog()
{
    delete _pUi;
}

int RegisterDialog::execWithMbcImport()
{
    showImportDialog();

    return exec();
}

void RegisterDialog::addRegister(GraphData graphData)
{
    _pGraphDataModel->add(graphData);
}

void RegisterDialog::addDefaultRegister()
{
    _pGraphDataModel->add();
}

void RegisterDialog::showImportDialog()
{
    MbcRegisterModel mbcRegisterModel(_pGraphDataModel);
    ImportMbcDialog importMbcDialog(_pGuiModel, _pGraphDataModel, &mbcRegisterModel, this);

    if (importMbcDialog.exec() == QDialog::Accepted)
    {
        QList<GraphData> regList = mbcRegisterModel.selectedRegisterList();

        if (regList.size() > 0)
        {
            _pGraphDataModel->add(regList);
        }
    }
}

void RegisterDialog::activatedCell(QModelIndex modelIndex)
{
    if (modelIndex.column() == GraphDataModel::column::COLOR)
    {
        if (modelIndex.row() < _pGraphDataModel->size())
        {
            // Let user pick color
            QColor color = QColorDialog::getColor(_pGraphDataModel->color(modelIndex.row()));

            if (color.isValid())
            {
                // Set color in model
                _pGraphDataModel->setData(modelIndex, color, Qt::EditRole);
            }
            else
            {
                // user aborted
            }
        }
    }
    else if (modelIndex.column() == GraphDataModel::column::EXPRESSION)
    {
        ExpressionsDialog exprDialog(_pGraphDataModel, modelIndex.row(), qobject_cast<QWidget *>(parent()));

        exprDialog.exec();
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
