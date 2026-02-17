#include "diagnosticdialog.h"
#include "ui_diagnosticdialog.h"

#include "importexport/diagnosticexporter.h"
#include "models/diagnosticfilter.h"
#include "models/diagnosticmodel.h"
#include "util/fileselectionhelper.h"
#include "util/scopelogging.h"
#include "util/util.h"

#include <QClipboard>
#include <QFileDialog>
#include <QModelIndex>
#include <QScrollBar>
#include <algorithm>

DiagnosticDialog::DiagnosticDialog(DiagnosticModel* pDiagnosticModel, QWidget* parent)
    : QDialog(parent), _pUi(new Ui::DiagnosticDialog)
{
    _pUi->setupUi(this);
    _bAutoScroll = false;

    _pDiagnosticModel = pDiagnosticModel;

    _pSeverityProxyFilter = new DiagnosticFilter();
    _pSeverityProxyFilter->setSourceModel(_pDiagnosticModel);

    // Create button group for filtering
    _categoryFilterGroup.setExclusive(false);
    _categoryFilterGroup.addButton(_pUi->checkInfo);
    _categoryFilterGroup.addButton(_pUi->checkWarning);
    _categoryFilterGroup.addButton(_pUi->checkDebug);

    connect(&_categoryFilterGroup, QOverload<int>::of(&QButtonGroup::idClicked), this, &DiagnosticDialog::handleFilterChange);

    this->handleFilterChange(); // Update filter

    _pUi->listError->setModel(_pSeverityProxyFilter);
    _pUi->listError->setUniformItemSizes(true); // For performance
    _pUi->listError->setSelectionMode(QAbstractItemView::MultiSelection);

    connect(_pUi->checkDebugLogs, &QCheckBox::checkStateChanged, this, &DiagnosticDialog::handleEnableDebugLog);

    connect(_pUi->listError->verticalScrollBar(), &QScrollBar::valueChanged, this, &DiagnosticDialog::handleScrollbarChange);

    // Disable auto scroll when selecting an item
    connect(_pUi->listError->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DiagnosticDialog::handleErrorSelectionChanged);

    // Handle layout change (filter change, add, clear, ...)
    connect(_pDiagnosticModel, &QAbstractItemModel::rowsInserted, this, &DiagnosticDialog::handleLogsChanged);
    connect(_pDiagnosticModel, &QAbstractItemModel::rowsRemoved, this, &DiagnosticDialog::handleLogsChanged);

    connect(_pUi->checkAutoScroll, &QCheckBox::checkStateChanged, this,
            &DiagnosticDialog::handleCheckAutoScrollChanged);

    connect(_pUi->pushClear, &QPushButton::clicked, this, &DiagnosticDialog::handleClearButton);
    connect(_pUi->pushExport, &QPushButton::clicked, this, &DiagnosticDialog::handleExportLog);

    if (_pDiagnosticModel->minimumSeverityLevel() >= Diagnostic::LOG_DEBUG)
    {
        _pUi->checkDebugLogs->setChecked(true);
    }
    else
    {
        _pUi->checkDebugLogs->setChecked(false);
    }

    // default to autoscroll
    setAutoScroll(true);

    // For rightclick menu
    _pDiagnosticMenu = new QMenu(this);
    _pCopyDiagnosticAction = _pDiagnosticMenu->addAction("Copy");
    _pCopyDiagnosticAction->setShortcut(QKeySequence::Copy);

    // Make the shortcut active while the dialog (or its children) has focus
    _pCopyDiagnosticAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    // Register the action with the dialog so WindowShortcut would also work
    this->addAction(_pCopyDiagnosticAction);

    connect(_pCopyDiagnosticAction, &QAction::triggered, this, &DiagnosticDialog::handleCopyDiagnostics);

    _pUi->listError->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_pUi->listError, &QWidget::customContextMenuRequested, this, &DiagnosticDialog::showContextMenu);
}

DiagnosticDialog::~DiagnosticDialog()
{
    delete _pUi;
}

void DiagnosticDialog::handleLogsChanged()
{
    updateScroll();

    updateLogCount();
}

void DiagnosticDialog::handleErrorSelectionChanged(QItemSelection selected, QItemSelection deselected)
{
    Q_UNUSED(deselected);

    /* Stop auto scroll when an item is selected */
    if (selected.size() != 0)
    {
        setAutoScroll(false);
    }
}

void DiagnosticDialog::handleCheckAutoScrollChanged(Qt::CheckState state)
{
    if (state == Qt::Unchecked)
    {
        setAutoScroll(false);
    }
    else
    {
        setAutoScroll(true);
    }

}

void DiagnosticDialog::handleScrollbarChange()
{
    const QScrollBar * pScroll = _pUi->listError->verticalScrollBar();
    if (pScroll->value() == pScroll->maximum())
    {
        setAutoScroll(true);

        _pUi->listError->clearSelection();
    }
}

void DiagnosticDialog::handleClearButton()
{
    _pDiagnosticModel->clear();
}

void DiagnosticDialog::handleFilterChange()
{
    quint32 bitmask = 0;

    if (_pUi->checkInfo->checkState())
    {
        bitmask |= 1 << Diagnostic::LOG_INFO;
    }

    if (_pUi->checkWarning->checkState())
    {
        bitmask |= 1 << Diagnostic::LOG_WARNING;
    }

    if (_pUi->checkDebug->checkState())
    {
        bitmask |= 1 << Diagnostic::LOG_DEBUG;
    }

    _pSeverityProxyFilter->setFilterBitmask(bitmask);

    handleLogsChanged();
}

void DiagnosticDialog::handleEnableDebugLog(Qt::CheckState state)
{
    if (state == Qt::Checked)
    {
        _pDiagnosticModel->setMinimumSeverityLevel(Diagnostic::LOG_DEBUG);

        _pUi->checkDebug->setChecked(true);
        _pUi->checkDebug->setEnabled(true);
        handleFilterChange();
    }
    else
    {
        _pUi->checkDebug->setChecked(false);
        _pUi->checkDebug->setEnabled(false);
        handleFilterChange();

        _pDiagnosticModel->setMinimumSeverityLevel(Diagnostic::LOG_INFO);
    }
}

void DiagnosticDialog::handleExportLog()
{
    QFileDialog dialog(this);
    FileSelectionHelper::configureFileDialog(&dialog,
                                             FileSelectionHelper::DIALOG_TYPE_SAVE,
                                             FileSelectionHelper::FILE_TYPE_LOG);

    QString selectedFile = FileSelectionHelper::showDialog(&dialog);
    if (!selectedFile.isEmpty())
    {
        QFile file(selectedFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&file);
            DiagnosticExporter diagExporter(_pDiagnosticModel);

            diagExporter.exportDiagnosticsFile(stream);
        }
        else
        {
            Util::showError(tr("Save to log file (%1) failed").arg(selectedFile));
        }
    }
}

void DiagnosticDialog::showContextMenu(const QPoint& pos)
{
    QPoint globalPos = _pUi->listError->viewport()->mapToGlobal(pos);
    _pDiagnosticMenu->popup(globalPos);
}

void DiagnosticDialog::handleCopyDiagnostics()
{
    QModelIndexList indexlist =_pUi->listError->selectionModel()->selectedRows();
    std::sort(indexlist.begin(), indexlist.end(), std::less<QModelIndex>());

    QString clipboardText;
    foreach (QModelIndex index, indexlist)
    {
        QModelIndex sourceIndex = _pSeverityProxyFilter->mapToSource(index);
        clipboardText.append(QString("%1\n").arg(_pDiagnosticModel->toString(sourceIndex.row())));
    }

    QClipboard* pClipboard = QGuiApplication::clipboard();
    pClipboard->setText(clipboardText);
}

void DiagnosticDialog::setAutoScroll(bool bAutoScroll)
{
    if (_bAutoScroll != bAutoScroll)
    {
        _bAutoScroll = bAutoScroll;

        _pUi->checkAutoScroll->setChecked(_bAutoScroll);

        updateScroll();
    }
}

void DiagnosticDialog::updateScroll()
{
    if (_bAutoScroll)
    {
        _pUi->listError->scrollToBottom();
    }
    else
    {
       // Don't auto scroll
    }
}

void DiagnosticDialog::updateLogCount()
{
    if (_pSeverityProxyFilter->rowCount())
    {
        _pUi->grpBoxLogs->setTitle(QString("Logs (%1/%2)").arg(_pSeverityProxyFilter->rowCount()).arg(_pDiagnosticModel->size()));
    }
    else
    {
        _pUi->grpBoxLogs->setTitle(QString("Logs"));
    }
}
