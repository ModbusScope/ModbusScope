
#include <QMessageBox>
#include <QColorDialog>

#include "registerdialog.h"
#include "importmbcdialog.h"
#include "ui_registerdialog.h"

RegisterDialog::RegisterDialog(GuiModel *pGuiModel, GraphDataModel * pGraphDataModel,  QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::RegisterDialog)
{
    _pUi->setupUi(this);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    _pGraphDataModel = pGraphDataModel;
    _pGuiModel = pGuiModel;

    // Setup registerView
    _pUi->registerView->setModel(_pGraphDataModel);
    _pUi->registerView->verticalHeader()->hide();

    /* Don't stretch columns */
    _pUi->registerView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    /* Except following columns */
    _pUi->registerView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);

    // Select using click, shift and control
    _pUi->registerView->setSelectionBehavior(QAbstractItemView::SelectRows);
    _pUi->registerView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Handle color cell active signal
    connect(_pUi->registerView, SIGNAL(activated(QModelIndex)), this, SLOT(activatedCell(QModelIndex)));

    // Handle delete
    QShortcut* shortcut = new QShortcut(QKeySequence(QKeySequence::Delete), _pUi->registerView);
    connect(shortcut, SIGNAL(activated()), this, SLOT(removeRegisterRow()));


    // Setup handler for buttons
    connect(_pUi->btnImportFromMbc, SIGNAL(released()), this, SLOT(showImportDialog()));
    connect(_pUi->btnAdd, SIGNAL(released()), this, SLOT(addRegisterRow()));
    connect(_pUi->btnRemove, SIGNAL(released()), this, SLOT(removeRegisterRow()));
    connect(_pGraphDataModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(onRegisterInserted(QModelIndex,int,int)));
}

RegisterDialog::~RegisterDialog()
{
    delete _pUi;
}

void RegisterDialog::done(int r)
{
    bool bValid = true;

    if(QDialog::Accepted == r)  // ok was pressed
    {
        quint16 duplicateReg = 0;
        quint16 duplicateBitMask = 0;
        if (!_pGraphDataModel->getDuplicate(&duplicateReg, &duplicateBitMask))
        {
            bValid = false;

            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Duplicate register!"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Register %1 with bitmask 0x%2 is defined twice in the list.").arg(duplicateReg).arg(duplicateBitMask, 0, 16));
            msgBox.exec();
        }
    }
    else
    {
        // cancel, close or exc was pressed
        bValid = true;
    }

    if (bValid)
    {
        QDialog::done(r);
    }
}

int RegisterDialog::exec()
{
    return QDialog::exec();
}

int RegisterDialog::exec(QString mbcFile)
{
    showImportDialog(mbcFile);

    return exec();
}

void RegisterDialog::addRegisterRow()
{
    _pGraphDataModel->insertRow(_pGraphDataModel->size());
}

void RegisterDialog::showImportDialog()
{
    showImportDialog(QString(""));
}

void RegisterDialog::showImportDialog(QString mbcPath)
{
    ImportMbcDialog importMbcDialog(_pGuiModel, _pGraphDataModel, this);

    if (importMbcDialog.exec(mbcPath) == QDialog::Accepted)
    {
        QList<GraphData> regList;

        importMbcDialog.selectedRegisterList(&regList);

        if (regList.size() > 0)
        {
            _pGraphDataModel->add(regList);
        }
    }
}

void RegisterDialog::activatedCell(QModelIndex modelIndex)
{
    if (modelIndex.column() == 0)
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
}

bool RegisterDialog::sortRegistersLastFirst(const QModelIndex &s1, const QModelIndex &s2)
{
    return s1.row() > s2.row();
}
