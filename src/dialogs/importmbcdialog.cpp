#include "importmbcdialog.h"
#include "ui_importmbcdialog.h"

#include "util.h"
#include "guimodel.h"
#include "mbcfileimporter.h"
#include "mbcregistermodel.h"
#include "graphdata.h"
#include "graphdatamodel.h"

#include <QFileDialog>

ImportMbcDialog::ImportMbcDialog(GuiModel * pGuiModel, GraphDataModel * pGraphDataModel, MbcRegisterModel * pMbcRegisterModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::ImportMbcDialog)
{
    _pUi->setupUi(this);

    _pGuiModel = pGuiModel;
    _pGraphDataModel = pGraphDataModel;
    _pMbcRegisterModel = pMbcRegisterModel;

    _pTabProxyFilter = new MbcRegisterFilter();
    _pTabProxyFilter->setSourceModel(_pMbcRegisterModel);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Setup registerView
    _pUi->tblMbcRegisters->setModel(_pTabProxyFilter);
    _pUi->tblMbcRegisters->setSortingEnabled(false);

    /* Don't stretch columns */
    _pUi->tblMbcRegisters->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    /* Except following columns */
    _pUi->tblMbcRegisters->horizontalHeader()->setSectionResizeMode(MbcRegisterModel::cColumnText, QHeaderView::Stretch);
    _pUi->tblMbcRegisters->horizontalHeader()->setSectionResizeMode(MbcRegisterModel::cColumnTab, QHeaderView::Stretch);

    // Select using click, shift and control
    _pUi->tblMbcRegisters->setSelectionBehavior(QAbstractItemView::SelectItems);
    _pUi->tblMbcRegisters->setSelectionMode(QAbstractItemView::NoSelection);

    _pUi->tblMbcRegisters->setFocusPolicy(Qt::NoFocus);

    connect(_pUi->btnSelectMbcFile, SIGNAL(clicked()), this, SLOT(selectMbcFile()));
    connect(_pMbcRegisterModel, &QAbstractItemModel::dataChanged, this, &ImportMbcDialog::registerDataChanged);

    connect(_pUi->cmbTabFilter, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), _pTabProxyFilter, &MbcRegisterFilter::setTab);
    connect(_pUi->lineTextFilter, &QLineEdit::textChanged, _pTabProxyFilter, &MbcRegisterFilter::setTextFilter);
}

ImportMbcDialog::~ImportMbcDialog()
{
    delete _pUi;
}

int ImportMbcDialog::exec(QString mbcPath)
{
    _mbcFilePath = mbcPath;
    _pUi->lineMbcfile->setText(_mbcFilePath);

    updateMbcRegisters();

    return QDialog::exec();
}

int ImportMbcDialog::exec(void)
{
    return exec(QString(""));
}

void ImportMbcDialog::selectMbcFile()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setOption(QFileDialog::HideNameFilterDetails, false);
    dialog.setDefaultSuffix("mbc");
    dialog.setWindowTitle(tr("Select mbc file"));
    dialog.setNameFilter(tr("MBC files (*.mbc)"));
    dialog.setDirectory(_pGuiModel->lastDir());

    if (dialog.exec() == QDialog::Accepted)
    {
        _mbcFilePath = dialog.selectedFiles().first();
        _pUi->lineMbcfile->setText(_mbcFilePath);

        _pGuiModel->setLastDir(QFileInfo(_mbcFilePath).dir().absolutePath());

        if (QFile(_mbcFilePath).exists())
        {
            if (updateMbcRegisters())
            {
                /* TODO: handle correctly */
            }
            else
            {

            }
        }
        else
        {
            Util::showError("No valid MBC file selected.");
        }
    }
}

void ImportMbcDialog::registerDataChanged()
{
    const quint32 count = _pMbcRegisterModel->selectedRegisterCount();
    if (count == 1)
    {
        _pUi->lblSelectedCount->setText(QString("You have selected %0 register.").arg(count));
    }
    else
    {
        _pUi->lblSelectedCount->setText(QString("You have selected %0 registers.").arg(count));
    }
}

bool ImportMbcDialog::updateMbcRegisters()
{
    bool bSuccess = false;

    MbcFileImporter fileImporter(_mbcFilePath);
    QList <MbcRegisterData> registerList = fileImporter.registerList();
    QStringList tabList = fileImporter.tabList();

    /* Clear data from table widget */
    _pMbcRegisterModel->reset();
    registerDataChanged();

    if (registerList.size() > 0)
    {
        _pMbcRegisterModel->fill(registerList, tabList);

        /* Update combo box */
        _pUi->cmbTabFilter->clear();
        _pUi->cmbTabFilter->addItem(MbcRegisterFilter::cTabNoFilter);
        _pUi->cmbTabFilter->addItems(tabList);

        /* Resize dialog window to fix table widget */
        // TODO

        bSuccess = true;
    }

    return bSuccess;
}

