#include "diagnosticdialog.h"
#include <QScrollBar>
#include <QFileDialog>
#include "ui_diagnosticdialog.h"

#include "guimodel.h"
#include "util.h"
#include "diagnosticmodel.h"
#include "diagnosticfilter.h"
#include "diagnosticexporter.h"
#include "scopelogging.h"

DiagnosticDialog::DiagnosticDialog(GuiModel* pGuiModel, DiagnosticModel * pDiagnosticModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::DiagnosticDialog)
{
    _pUi->setupUi(this);

    _pDiagnosticModel = pDiagnosticModel;
    _pGuiModel = pGuiModel;

    _pSeverityProxyFilter = new DiagnosticFilter();
    _pSeverityProxyFilter->setSourceModel(_pDiagnosticModel);

    // Create button group for filtering
    _categoryFilterGroup.setExclusive(false);
    _categoryFilterGroup.addButton(_pUi->checkInfo);
    _categoryFilterGroup.addButton(_pUi->checkWarning);
    _categoryFilterGroup.addButton(_pUi->checkDebug);

    connect(&_categoryFilterGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &DiagnosticDialog::handleFilterChange);
    this->handleFilterChange(0); // Update filter

    _pUi->listError->setModel(_pSeverityProxyFilter);
    _pUi->listError->setUniformItemSizes(true); // For performance
    _pUi->listError->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(_pUi->checkDebugLogs, &QCheckBox::stateChanged, this, &DiagnosticDialog::handleEnableDebugLog);

    connect(_pUi->listError->verticalScrollBar(), &QScrollBar::valueChanged, this, &DiagnosticDialog::handleScrollbarChange);

    // Disable auto scroll when selecting an item
    connect(_pUi->listError->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DiagnosticDialog::handleErrorSelectionChanged);

    // Handle inserted row
    connect(_pUi->listError->model(), &QAbstractItemModel::rowsInserted, this, &DiagnosticDialog::handleLogsChanged);
    connect(_pUi->listError->model(), &QAbstractItemModel::rowsRemoved, this, &DiagnosticDialog::handleLogsChanged);

    connect(_pUi->checkAutoScroll, &QCheckBox::stateChanged, this, &DiagnosticDialog::handleCheckAutoScrollChanged);

    connect(_pUi->pushClear, &QPushButton::clicked, this, &DiagnosticDialog::handleClearButton);
    connect(_pUi->pushExport, &QPushButton::clicked, this, &DiagnosticDialog::handleExportLog);

    // default to autoscroll
    setAutoScroll(true);
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

void DiagnosticDialog::handleCheckAutoScrollChanged(int newState)
{
    if (newState == Qt::Unchecked)
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

void DiagnosticDialog::handleFilterChange(int id)
{
    Q_UNUSED(id);

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

    updateLogCount();
}

void DiagnosticDialog::handleEnableDebugLog(int state)
{
    if (state == Qt::Checked)
    {
        _pUi->checkDebug->setEnabled(true);
        ScopeLogging::Logger().setMinimumSeverityLevel(Diagnostic::LOG_DEBUG);
    }
    else
    {
        _pUi->checkDebug->setEnabled(false);
        ScopeLogging::Logger().setMinimumSeverityLevel(Diagnostic::LOG_INFO);
    }
}

void DiagnosticDialog::handleExportLog()
{
    QString filePath;
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::HideNameFilterDetails, false);
    dialog.setDefaultSuffix("log");
    dialog.setWindowTitle(tr("Select log file"));
    dialog.setNameFilter(tr("LOG files (*.log)"));
    dialog.setDirectory(_pGuiModel->lastDir());

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        _pGuiModel->setLastDir(QFileInfo(filePath).dir().absolutePath());

        QFile file(filePath);
        if (file.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream stream(&file);
            DiagnosticExporter diagExporter(_pDiagnosticModel);

            diagExporter.exportDiagnosticsFile(stream);
        }
        else
        {
            Util::showError(tr("Save to log file (%1) failed").arg(filePath));
        }
    }
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
    if (_pUi->checkInfo->checkState() == Qt::Checked || _pUi->checkWarning->checkState() == Qt::Checked)
    {
        _pUi->grpBoxLogs->setTitle(QString("Logs (%1/%2)").arg(_pSeverityProxyFilter->rowCount()).arg(_pDiagnosticModel->size()));
    }
    else
    {
        _pUi->grpBoxLogs->setTitle(QString("Logs"));
    }
}
