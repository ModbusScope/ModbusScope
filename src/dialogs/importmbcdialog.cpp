#include "importmbcdialog.h"

#include "customwidgets/actionbuttondelegate.h"
#include "dialogs/mbcheader.h"
#include "dialogs/ui_importmbcdialog.h"
#include "importexport/mbcfileimporter.h"
#include "models/graphdatamodel.h"
#include "models/guimodel.h"
#include "models/mbcregisterfilter.h"
#include "models/mbcregistermodel.h"
#include "models/mbcupdatemodel.h"
#include "util/fileselectionhelper.h"
#include "util/util.h"

#include <QFileDialog>

ImportMbcDialog::ImportMbcDialog(GuiModel* pGuiModel, GraphDataModel* pGraphDatamodel, QWidget* parent)
    : QDialog(parent), _pUi(new Ui::ImportMbcDialog), _pGuiModel(pGuiModel), _pGraphDataModel(pGraphDatamodel)
{
    _pUi->setupUi(this);

    _pMbcUpdateModel = new MbcUpdateModel(_pGraphDataModel);

    _pTabProxyFilter = new MbcRegisterFilter();
    _pTabProxyFilter->setSourceModel(&_mbcRegisterModel);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Setup registerView
    _pUi->tblMbcRegisters->setModel(_pTabProxyFilter);
    _pUi->tblMbcRegisters->setSortingEnabled(false);

    auto mbcHeader = new MbcHeader(Qt::Horizontal, _pUi->tblMbcRegisters);
    _pUi->tblMbcRegisters->setHorizontalHeader(mbcHeader);

    /* Don't stretch columns */
    _pUi->tblMbcRegisters->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    /* Except following columns */
    _pUi->tblMbcRegisters->horizontalHeader()->setSectionResizeMode(MbcRegisterModel::cColumnText,
                                                                    QHeaderView::Stretch);
    _pUi->tblMbcRegisters->horizontalHeader()->setSectionResizeMode(MbcRegisterModel::cColumnTab, QHeaderView::Stretch);

    // Select using click, shift and control
    _pUi->tblMbcRegisters->setSelectionBehavior(QAbstractItemView::SelectItems);
    _pUi->tblMbcRegisters->setSelectionMode(QAbstractItemView::NoSelection);

    _pUi->tblMbcRegisters->setFocusPolicy(Qt::NoFocus);

    _pUi->tblMbcRegisters->setStyle(&_centeredBoxStyle);

    // setup register
    _pUi->tblMbcUpdate->setModel(_pMbcUpdateModel);
    _pUi->tblMbcUpdate->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Select using click, shift and control
    _pUi->tblMbcUpdate->setSelectionBehavior(QAbstractItemView::SelectItems);
    _pUi->tblMbcUpdate->setSelectionMode(QAbstractItemView::NoSelection);

    _pUi->tblMbcUpdate->setFocusPolicy(Qt::NoFocus);

    _pUpdateDelegate = std::make_unique<ActionButtonDelegate>(_pUi->tblMbcUpdate);
    _pUpdateDelegate->setCharacter(QChar(0x2713));
    connect(_pUpdateDelegate.get(), &ActionButtonDelegate::clicked, this, &ImportMbcDialog::handleAcceptUpdate);

    _pUi->tblMbcUpdate->setItemDelegateForColumn(MbcUpdateModel::cColumnUpdateExpression, _pUpdateDelegate.get());
    _pUi->tblMbcUpdate->setItemDelegateForColumn(MbcUpdateModel::cColumnUpdateText, _pUpdateDelegate.get());

    connect(_pUi->btnSelectMbcFile, &QToolButton::clicked, this, &ImportMbcDialog::selectMbcFile);
    connect(_pUi->btnImportRegisters, &QPushButton::clicked, this, &ImportMbcDialog::importSelectedRegisters);
    connect(&_mbcRegisterModel, &QAbstractItemModel::dataChanged, this, &ImportMbcDialog::registerDataChanged);
    connect(_pTabProxyFilter, &MbcRegisterFilter::dataChanged, this, &ImportMbcDialog::visibleItemsDataChanged);
    connect(mbcHeader, &MbcHeader::selectAllClicked, this, &ImportMbcDialog::handleSelectAllClicked);

    connect(_pUi->cmbTabFilter, &QComboBox::currentTextChanged, _pTabProxyFilter, &MbcRegisterFilter::setTab);
    connect(_pUi->lineTextFilter, &QLineEdit::textChanged, this, &ImportMbcDialog::updateTextFilter);
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

void ImportMbcDialog::updateTextFilter()
{
    auto checkHeight = _pUi->tblMbcRegisters->rowHeight(0) / 2;
    QModelIndex topRow = _pUi->tblMbcRegisters->indexAt(QPoint(checkHeight, checkHeight));

    auto currentTopModelIndex = _pTabProxyFilter->mapToSource(topRow);
    _pTabProxyFilter->setTextFilter(_pUi->lineTextFilter->text());

    auto newTopModelIndex = _pTabProxyFilter->mapFromSource(currentTopModelIndex);

    _pUi->tblMbcRegisters->scrollTo(newTopModelIndex, QAbstractItemView::PositionAtTop);
}

void ImportMbcDialog::selectMbcFile()
{
    QFileDialog dialog(this);
    FileSelectionHelper::configureFileDialog(&dialog, FileSelectionHelper::DIALOG_TYPE_OPEN,
                                             FileSelectionHelper::FILE_TYPE_MBC);

    QString selectedFile = FileSelectionHelper::showDialog(&dialog);
    if (!selectedFile.isEmpty())
    {
        _pUi->lineMbcfile->setText(selectedFile);
        _pGuiModel->setLastMbcImportedFile(selectedFile);

        updateMbcRegisters(selectedFile);
    }
}

void ImportMbcDialog::importSelectedRegisters()
{
    QList<GraphData> regList = _mbcRegisterModel.selectedRegisterList();

    if (!regList.isEmpty())
    {
        _pGraphDataModel->add(regList);
    }

    setSelectedSelectionstate(Qt::Unchecked);
}

void ImportMbcDialog::visibleItemsDataChanged()
{
    const int regCount = _pTabProxyFilter->rowCount();

    bool checked = true;
    bool unchecked = true;

    for (int idx = 0; idx < regCount; idx++)
    {
        QModelIndex idxOfItem = _pTabProxyFilter->index(idx, MbcRegisterModel::cColumnSelected);
        if (_pTabProxyFilter->data(idxOfItem, Qt::CheckStateRole) == Qt::Checked)
        {
            unchecked = false;
        }

        if (_pTabProxyFilter->data(idxOfItem, Qt::CheckStateRole) == Qt::Unchecked)
        {
            checked = false;
        }
    }

    Qt::CheckState selectAllState;
    if (checked)
    {
        selectAllState = Qt::Checked;
    }
    else if (unchecked)
    {
        selectAllState = Qt::Unchecked;
    }
    else
    {
        selectAllState = Qt::PartiallyChecked;
    }

    _mbcRegisterModel.setHeaderData(MbcRegisterModel::cColumnSelected, Qt::Horizontal, selectAllState,
                                    Qt::CheckStateRole);
}

void ImportMbcDialog::registerDataChanged()
{
    const quint32 count = _mbcRegisterModel.selectedRegisterCount();
    if (count == 1)
    {
        _pUi->lblSelectedCount->setText(QString("You have selected %0 register.").arg(count));
    }
    else
    {
        _pUi->lblSelectedCount->setText(QString("You have selected %0 registers.").arg(count));
    }
}

void ImportMbcDialog::handleSelectAllClicked(Qt::CheckState state)
{
    if (state == Qt::Unchecked)
    {
        setSelectedSelectionstate(Qt::Unchecked);
    }
    else if (state == Qt::Checked)
    {
        setSelectedSelectionstate(Qt::Checked);
    }
    else
    {
        // No need to handle PartialChecked
    }
}

void ImportMbcDialog::setSelectedSelectionstate(Qt::CheckState state)
{
    QList<QModelIndex> indexList;
    for (int idx = 0; idx < _pTabProxyFilter->rowCount(); idx++)
    {
        QModelIndex idxOfItem = _pTabProxyFilter->index(idx, MbcRegisterModel::cColumnSelected);
        indexList.append(_pTabProxyFilter->mapToSource(idxOfItem));
    }

    _mbcRegisterModel.setSelectionstate(indexList, state);
}

void ImportMbcDialog::updateMbcRegisters(QString filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        QString mbcFileContent = in.readAll();
        MbcFileImporter fileImporter(&mbcFileContent);
        QList<MbcRegisterData> registerList = fileImporter.registerList();
        QStringList tabList = fileImporter.tabList();

        /* Clear data from table widget */
        _mbcRegisterModel.reset();
        registerDataChanged();

        if (registerList.size() > 0)
        {
            _mbcRegisterModel.fill(registerList, tabList);
            _pMbcUpdateModel->setMbcRegisters(registerList);

            /* Update combo box */
            _pUi->cmbTabFilter->clear();
            _pUi->cmbTabFilter->addItem(MbcRegisterFilter::cTabNoFilter);
            _pUi->cmbTabFilter->addItems(tabList);
        }
    }
    else
    {
        Util::showError(tr("The file (\"%1\") can't be read.").arg(filePath));
    }
}

void ImportMbcDialog::handleAcceptUpdate(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }

    if (index.column() == MbcUpdateModel::cColumnUpdateExpression)
    {
        _pGraphDataModel->setExpression(index.row(), _pMbcUpdateModel->data(index).toString());
    }
    else if (index.column() == MbcUpdateModel::cColumnUpdateText)
    {
        _pGraphDataModel->setLabel(index.row(), _pMbcUpdateModel->data(index).toString());
    }
    else
    {
        // nothing to do
    }
}
