#include "diagnosticdialog.h"
#include <QScrollBar>
#include "ui_diagnosticdialog.h"

#include "diagnosticmodel.h"
#include "diagnosticfilter.h"

DiagnosticDialog::DiagnosticDialog(QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::DiagnosticDialog)
{
    _pUi->setupUi(this);

    _pSeverityProxyFilter = new DiagnosticFilter();
    _pSeverityProxyFilter->setSourceModel(&DiagnosticModel::Logger());

    // Create button group for filtering
    _categoryFilterGroup.setExclusive(false);
    _categoryFilterGroup.addButton(_pUi->checkInfo);
    _categoryFilterGroup.addButton(_pUi->checkError);
    _categoryFilterGroup.addButton(_pUi->checkDebug);
    connect(&_categoryFilterGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &DiagnosticDialog::handleFilterChange);
    this->handleFilterChange(0); // Update filter

    _pUi->listError->setModel(_pSeverityProxyFilter);
    _pUi->listError->setUniformItemSizes(true); // For performance
    _pUi->listError->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(_pUi->listError->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(handleScrollbarChange()));

    // Disable auto scroll when selecting an item
    connect(_pUi->listError->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(handleErrorSelectionChanged(QItemSelection,QItemSelection)));

    // Handle inserted row
    connect(_pUi->listError->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(handleLogsChanged()));
    connect(_pUi->listError->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(handleLogsChanged()));

    connect(_pUi->checkAutoScroll, SIGNAL(stateChanged(int)), this, SLOT(handleCheckAutoScrollChanged(int)));

    connect(_pUi->pushClear, SIGNAL(clicked(bool)), this, SLOT(handleClearButton()));

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
    DiagnosticModel::Logger().clear();
}

void DiagnosticDialog::handleFilterChange(int id)
{
    Q_UNUSED(id);

    quint32 bitmask = 0;

    if (_pUi->checkInfo->checkState())
    {
        bitmask |= 1 << Diagnostic::LOG_INFO;
    }

    if (_pUi->checkError->checkState())
    {
        bitmask |= 1 << Diagnostic::LOG_ERROR;
    }

    if (_pUi->checkDebug->checkState())
    {
        bitmask |= 1 << Diagnostic::LOG_DEBUG;
    }

    _pSeverityProxyFilter->setFilterBitmask(bitmask);

    updateLogCount();
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
    if (_pUi->checkInfo->checkState() == Qt::Checked || _pUi->checkError->checkState() == Qt::Checked)
    {
        _pUi->grpBoxLogs->setTitle(QString("Logs (%1/%2)").arg(_pSeverityProxyFilter->rowCount()).arg(DiagnosticModel::Logger().size()));
    }
    else
    {
        _pUi->grpBoxLogs->setTitle(QString("Logs"));
    }
}
