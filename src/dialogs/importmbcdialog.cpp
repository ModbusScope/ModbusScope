#include "importmbcdialog.h"
#include "ui_importmbcdialog.h"

#include "util.h"
#include "guimodel.h"
#include "mbcfileimporter.h"
#include "mbcregistermodel.h"
#include "graphdata.h"
#include "graphdatamodel.h"
#include "fileselectionhelper.h"

#include <QFileDialog>
#include <QFile>

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

    connect(_pUi->btnSelectMbcFile, &QToolButton::clicked, this, &ImportMbcDialog::selectMbcFile);
    connect(_pMbcRegisterModel, &QAbstractItemModel::dataChanged, this, &ImportMbcDialog::registerDataChanged);

    connect(_pUi->cmbTabFilter, &QComboBox::currentTextChanged, _pTabProxyFilter, &MbcRegisterFilter::setTab);
    connect(_pUi->lineTextFilter, &QLineEdit::textChanged, _pTabProxyFilter, &MbcRegisterFilter::setTextFilter);
}

ImportMbcDialog::~ImportMbcDialog()
{
    delete _pUi;
}

int ImportMbcDialog::exec()
{
    QString filePath = _pGuiModel->lastMbcImportedFile();
    if (!filePath.isEmpty())
    {
        /* Auto load with supplied path */
        updateMbcRegisters(filePath);

        _pUi->lineMbcfile->setText(filePath);
    }
    else
    {
        /* Skip auto load: no file path */
        _pUi->lineMbcfile->setText("");
    }

    return QDialog::exec();
}

void ImportMbcDialog::selectMbcFile()
{
    QFileDialog dialog(this);
    FileSelectionHelper::configureFileDialog(&dialog,
                                             FileSelectionHelper::DIALOG_TYPE_OPEN,
                                             FileSelectionHelper::FILE_TYPE_MBC);

    if (dialog.exec() == QDialog::Accepted)
    {
        auto fileList = dialog.selectedFiles();
        if (!fileList.isEmpty())
        {
            QString filePath = fileList.at(0);

            _pUi->lineMbcfile->setText(filePath);

            _pGuiModel->setLastDir(QFileInfo(filePath).dir().absolutePath());
            _pGuiModel->setLastMbcImportedFile(filePath);

            updateMbcRegisters(filePath);
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

void ImportMbcDialog::updateMbcRegisters(QString filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        QString mbcFileContent = in.readAll();
        MbcFileImporter fileImporter(&mbcFileContent);
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
        }
    }
    else
    {
        Util::showError(tr("The file (\"%1\") can't be read.").arg(filePath));
    }
}

