#include "errorlogdialog.h"
#include <QScrollBar>
#include "ui_errorlogdialog.h"

#include "errorlogmodel.h"

ErrorLogDialog::ErrorLogDialog(ErrorLogModel * pErrorLogModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::ErrorLogDialog)
{
    _pUi->setupUi(this);

    _pErrorLogModel = pErrorLogModel;

    _pUi->listError->setModel(_pErrorLogModel);
    _pUi->listError->setUniformItemSizes(true); // For performance
    _pUi->listError->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(_pUi->listError->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(handleScrollbarChange()));

    // Disable auto scroll when selecting an item
    connect(_pUi->listError->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(handleErrorSelectionChanged(QItemSelection,QItemSelection)));

    // Handle inserted row
    connect(_pUi->listError->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(handleLogsInserted()));

    connect(_pUi->checkAutoScroll, SIGNAL(stateChanged(int)), this, SLOT(handleCheckAutoScrollChanged(int)));

    // default to autoscroll
    setAutoScroll(true);
}

ErrorLogDialog::~ErrorLogDialog()
{
    delete _pUi;
}

void ErrorLogDialog::handleLogsInserted()
{
    updateScroll();
}

void ErrorLogDialog::handleErrorSelectionChanged(QItemSelection selected, QItemSelection deselected)
{
    Q_UNUSED(deselected);

    /* Stop auto scroll when an item is selected */
    if (selected.size() != 0)
    {
        setAutoScroll(false);
    }
}

void ErrorLogDialog::handleCheckAutoScrollChanged(int newState)
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

void ErrorLogDialog::handleScrollbarChange()
{
    const QScrollBar * pScroll = _pUi->listError->verticalScrollBar();
    if (pScroll->value() == pScroll->maximum())
    {
        setAutoScroll(true);

        _pUi->listError->clearSelection();
    }
}

void ErrorLogDialog::setAutoScroll(bool bAutoScroll)
{
    if (_bAutoScroll != bAutoScroll)
    {
        _bAutoScroll = bAutoScroll;

        _pUi->checkAutoScroll->setChecked(_bAutoScroll);

        updateScroll();
    }
}

void ErrorLogDialog::updateScroll()
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
