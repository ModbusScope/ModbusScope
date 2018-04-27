

#include <QStyleOption>
#include <QDebug>
#include <QPainter>
#include <QMouseEvent>

#include "guimodel.h"
#include "graphdatamodel.h"
#include "util.h"
#include "legend.h"
#include "basicgraphview.h"

Legend::Legend(QWidget *parent) : QFrame(parent)
{
    _pLayout = new QVBoxLayout();

    _pNoGraphs = new QLabel("No active graphs");
    _pLegendTable = new QTableWidget(this);
    _pLegendTable->setRowCount(0);
    _pLegendTable->setColumnCount(2);
    _pLegendTable->horizontalHeader()->setStretchLastSection(true);
    _pLegendTable->setColumnWidth(0,50);
    _pLegendTable->setShowGrid(false);
    _pLegendTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _pLegendTable->setFocusPolicy(Qt::NoFocus);
    _pLegendTable->setSelectionMode(QAbstractItemView::NoSelection);
    _pLegendTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    _pLegendTable->setHorizontalHeaderLabels(QStringList()<<"Value"<<"Register");
    _pLegendTable->hide();

    _pLayout->setSpacing(0);
    _pLayout->setContentsMargins(0, 0, 0, 0); // This is redundant with setMargin, which is deprecated

    _pLayout->addWidget(_pNoGraphs);
    _pLayout->addWidget(_pLegendTable);
    setLayout(_pLayout);

    // For rightclick menu
    _pLegendMenu = new QMenu(parent);

    _pToggleVisibilityAction = _pLegendMenu->addAction("Toggle item visibility");
    _pToggleVisibilityAction->setEnabled(false);

    (void)_pLegendMenu->addSeparator();
    _pHideAllAction = _pLegendMenu->addAction("Hide all");
    _pHideAllAction->setEnabled(false);

    _pShowAllAction = _pLegendMenu->addAction("Show all");
    _pShowAllAction->setEnabled(false);

    connect(_pToggleVisibilityAction, &QAction::triggered, this, &Legend::toggleVisibilityClicked);
    connect(_pHideAllAction, &QAction::triggered, this, &Legend::hideAll);
    connect(_pShowAllAction, &QAction::triggered, this, &Legend::showAll);
    connect(_pLegendTable, &QTableWidget::cellClicked, this, &Legend::graphToForeground);
    connect(_pLegendTable, &QTableWidget::cellDoubleClicked, this, &Legend::toggleGraphVisibility);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &Legend::customContextMenuRequested, this, &Legend::showContextMenu);
}

Legend::~Legend()
{
    delete _pLegendMenu;
}

void Legend::setGraphview(BasicGraphView * pGraphView)
{
    _pGraphView = pGraphView;
}

void Legend::setModels(GuiModel *pGuiModel, GraphDataModel * pGraphDataModel)
{
    _pGuiModel = pGuiModel;
    _pGraphDataModel = pGraphDataModel;

    connect(_pGraphDataModel, SIGNAL(activeChanged(quint32)), this, SLOT(updateLegend()));
    connect(_pGraphDataModel, SIGNAL(added(quint32)), this, SLOT(updateLegend()));
    connect(_pGraphDataModel, SIGNAL(removed(quint32)), this, SLOT(updateLegend()));
    connect(_pGraphDataModel, SIGNAL(visibilityChanged(quint32)), this, SLOT(showGraph(const quint32)));
    connect(_pGraphDataModel, SIGNAL(colorChanged(quint32)), this, SLOT(changeGraphColor(quint32)));
    connect(_pGraphDataModel, SIGNAL(labelChanged(quint32)), this, SLOT(changeGraphLabel(quint32)));

}

void Legend::graphToForeground(int row)
{
    _pGuiModel->setFrontGraph(row);
}

void Legend::toggleGraphVisibility(int row)
{
    toggleItemVisibility(row);
}

void Legend::addLastReceivedDataToLegend(QList<bool> successList, QList<double> valueList)
{
    _lastReceivedValueList.clear();

    for (qint32 i = 0; i < valueList.size(); i++)
    {
        if (successList[i])
        {
            // No error
            _lastReceivedValueList.append(QString("%1").arg(Util::formatDoubleForExport(valueList[i])));
        }
        else
        {
            /* Show error */
            _lastReceivedValueList.append("-");
        }
    }

    updateDataInLegend();
}

void Legend::updateDataInLegend()
{
    QStringList legendDataValues;

    /* Select correct values to show */
    if (_pGuiModel->cursorValues())
    {
        updateCursorDataInLegend(legendDataValues);
    }
    else
    {
        legendDataValues = _lastReceivedValueList;
    }


    if (_pLegendTable->rowCount() == legendDataValues.size())
    {
        for (qint32 i = 0; i < legendDataValues.size(); i++)
        {
            _pLegendTable->item(i,0)->setText(legendDataValues[i]);
        }
    }
}

void Legend::updateLegend()
{

    if (_pGraphDataModel->activeCount() != 0)
    {
        QList<quint16> activeList;

        _pGraphDataModel->activeGraphIndexList(&activeList);
        _pLegendTable->setRowCount(0);

        for (qint32 idx = 0; idx < activeList.size(); idx++)
        {
            addItem(activeList[idx]);
        }

        _pNoGraphs->hide();
        _pLegendTable->show();

        _pToggleVisibilityAction->setEnabled(true);
        _pHideAllAction->setEnabled(true);
        _pShowAllAction->setEnabled(true);
    }
    else
    {
        _pNoGraphs->show();
        _pLegendTable->hide();

        _pToggleVisibilityAction->setEnabled(false);
        _pHideAllAction->setEnabled(false);
        _pShowAllAction->setEnabled(false);
    }
}

void Legend::showGraph(quint32 graphIdx)
{
    const qint32 activeGraphIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

    if (activeGraphIdx != -1)
    {

        QFont itemFont = _pLegendTable->item( int (graphIdx), 0)->font();
        QColor foreGroundColor = Qt::black;

        if (_pGraphDataModel->isVisible(graphIdx))
        {
            foreGroundColor = Qt::black;
            itemFont.setItalic(false);

            _pLegendTable->verticalHeaderItem(int (activeGraphIdx))->setBackground(_pGraphDataModel->color(graphIdx));
        }
        else
        {
            foreGroundColor = Qt::gray;
            itemFont.setItalic(true);

            _pLegendTable->verticalHeaderItem(int (activeGraphIdx))->setBackground(Qt::gray);
        }

        _pLegendTable->item( int (graphIdx), 0)->setFont(itemFont);
        _pLegendTable->item( int (graphIdx), 1)->setFont(itemFont);

        _pLegendTable->item( int (graphIdx), 0)->setForeground(foreGroundColor);
        _pLegendTable->item( int (graphIdx), 1)->setForeground(foreGroundColor);
    }
}

void Legend::changeGraphColor(const quint32 graphIdx)
{
    const qint32 activeGraphIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

    if (activeGraphIdx != -1)
    {
         _pLegendTable->verticalHeaderItem( int (activeGraphIdx))->setBackground(_pGraphDataModel->color(graphIdx));

    }
}

void Legend::changeGraphLabel(const quint32 graphIdx)
{
    const qint32 activeGraphIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

    if (activeGraphIdx != -1)
    {
       _pLegendTable->item( int (activeGraphIdx), 1)->setText(_pGraphDataModel->label(graphIdx));
    }
}

void Legend::updateCursorDataInLegend(QStringList &cursorValueList)
{
    QList<double> valueList;
    const bool bInRange = _pGraphView->valuesUnderCursor(valueList);

    cursorValueList.clear();

    for (qint32 i = 0; i < valueList.size(); i++)
    {
        if (bInRange)
        {
            // No error
            cursorValueList.append(QString("[%1]").arg(Util::formatDoubleForExport(valueList[i])));
        }
        else
        {
            /* Show error */
            cursorValueList.append("?");
        }
    }
}

void Legend::addItem(quint32 graphIdx)
{
    int row = _pLegendTable->rowCount();
    _pLegendTable->insertRow(row);
    _pLegendTable->setVerticalHeaderItem(row, new QTableWidgetItem());
    _pLegendTable->verticalHeaderItem(row)->setBackgroundColor(_pGraphDataModel->color(graphIdx));
    _pLegendTable->setItem(row, 0, new QTableWidgetItem("-") );
    _pLegendTable->item(row,0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    _pLegendTable->setItem(row, 1, new QTableWidgetItem(_pGraphDataModel->label(graphIdx)) );
}

void Legend::toggleItemVisibility(qint32 idx)
{
    if (idx != -1)
    {
        const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(idx);

        _pGraphDataModel->setVisible(graphIdx, !_pGraphDataModel->isVisible(graphIdx));
    }
}

void Legend::showContextMenu(const QPoint& pos)
{
    _popupMenuItem = _pLegendTable->currentRow();

    if (_popupMenuItem == -1)
    {
        _pToggleVisibilityAction->setEnabled(false);
    }
    else
    {
        _pToggleVisibilityAction->setEnabled(true);
    }

    _pLegendMenu->popup(mapToGlobal(pos));
}

void Legend::toggleVisibilityClicked()
{
    toggleItemVisibility(_popupMenuItem);
}

void Legend::hideAll()
{
    for(qint32 idx = 0; idx < _pGraphDataModel->size(); idx++)
    {
        const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(idx);
        _pGraphDataModel->setVisible(graphIdx, false);
    }
}

void Legend::showAll()
{
    for(qint32 idx = 0; idx < _pGraphDataModel->size(); idx++)
    {
        const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(idx);
        _pGraphDataModel->setVisible(graphIdx, true);
    }
}
