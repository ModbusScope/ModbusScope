

#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>
#include <QColorDialog>

#include "guimodel.h"
#include "graphdatamodel.h"
#include "util.h"
#include "legend.h"
#include "graphview.h"

Legend::Legend(QWidget *parent) : QFrame(parent)
{
    _pLayout = new QVBoxLayout();

    _pNoGraphs = new QLabel("No active graphs");
    _pLegendTable = new QTableWidget(this);
    _pLegendTable->setRowCount(0);
    _pLegendTable->setColumnCount(3);

    _pLegendTable->verticalHeader()->setDefaultSectionSize(_pLegendTable->verticalHeader()->fontMetrics().height()+2);
    _pLegendTable->verticalHeader()->hide();

    _pLegendTable->setShowGrid(false);
    _pLegendTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _pLegendTable->setFocusPolicy(Qt::NoFocus);
    _pLegendTable->setSelectionMode(QAbstractItemView::NoSelection);
    _pLegendTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    _pLegendTable->setHorizontalHeaderLabels(QStringList()<<" "<<"Value"<<"Register");
    _pLegendTable->hide();

    QHeaderView * horizontalHeader = _pLegendTable->horizontalHeader();
    QFontMetrics fontMetric = _pLegendTable->horizontalHeader()->fontMetrics();

    horizontalHeader->setMinimumSectionSize(fontMetric.boundingRect("X").width());
    horizontalHeader->setSectionResizeMode(QHeaderView::Interactive);

    /* Set default size of columns */
    horizontalHeader->resizeSection(cColummnColor, fontMetric.boundingRect("X").width());
    horizontalHeader->resizeSection(cColummnValue, fontMetric.boundingRect("[-0000000]").width());

    /* stretch text column */
    horizontalHeader->setSectionResizeMode(cColummnText, QHeaderView::Stretch);

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
    connect(_pLegendTable, &QTableWidget::cellDoubleClicked, this, &Legend::legendCellDoubleClicked);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &Legend::customContextMenuRequested, this, &Legend::showContextMenu);
}

Legend::~Legend()
{
    delete _pLegendMenu;
}

void Legend::setGraphview(GraphView * pGraphView)
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

void Legend::clearLegendData()
{
    for (qint32 i = 0; i < _lastReceivedValueList.size(); i++)
    {
        _lastReceivedValueList[i] = "-";
    }

    updateDataInLegend();
}

void Legend::graphToForeground(int row)
{
    _pGuiModel->setFrontGraph(row);
}

void Legend::legendCellDoubleClicked(int row, int column)
{
    if (column == 0)
    {
        if (row != -1)
        {
            const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(static_cast<quint32>(row));

            if (_pGraphDataModel->isVisible(graphIdx))
            {
                /* Color column */
                QColor color = QColorDialog::getColor(_pGraphDataModel->color(graphIdx));

                if (color.isValid())
                {
                    // Set color in model
                    _pGraphDataModel->setColor(graphIdx, color);
                }
                else
                {
                    // user aborted
                }
            }
        }
    }
    else
    {
        /* Other columns */
        toggleItemVisibility(row);
    }
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
            _pLegendTable->item(i,cColummnValue)->setText(legendDataValues[i]);
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
        QFont itemFont = _pLegendTable->item((int)activeGraphIdx, cColummnValue)->font();
        QColor foreGroundColor = Qt::black;
        QColor graphColor;

        if (_pGraphDataModel->isVisible(graphIdx))
        {
            foreGroundColor = Qt::black;
            itemFont.setItalic(false);

            graphColor = _pGraphDataModel->color(graphIdx);
        }
        else
        {
            foreGroundColor = Qt::gray;
            itemFont.setItalic(true);

            graphColor = Qt::white;
        }

        _pLegendTable->item((int)activeGraphIdx, cColummnValue)->setFont(itemFont);
        _pLegendTable->item((int)activeGraphIdx, cColummnText)->setFont(itemFont);

        _pLegendTable->item((int)activeGraphIdx, cColummnColor)->setBackground(graphColor);
        _pLegendTable->item((int)activeGraphIdx, cColummnValue)->setForeground(foreGroundColor);
        _pLegendTable->item((int)activeGraphIdx, cColummnText)->setForeground(foreGroundColor);
    }
}

void Legend::changeGraphColor(const quint32 graphIdx)
{
    const qint32 activeGraphIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

    if (activeGraphIdx != -1)
    {
        _pLegendTable->item((int)activeGraphIdx, cColummnColor)->setBackground(_pGraphDataModel->color(graphIdx));
    }
}

void Legend::changeGraphLabel(const quint32 graphIdx)
{
    const qint32 activeGraphIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

    if (activeGraphIdx != -1)
    {
       _pLegendTable->item((int)activeGraphIdx, cColummnText)->setText(_pGraphDataModel->label(graphIdx));
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

    _pLegendTable->setItem(row, cColummnColor, new QTableWidgetItem(""));
    _pLegendTable->item(row, cColummnColor)->setBackground(_pGraphDataModel->color(graphIdx));

    _pLegendTable->setItem(row, cColummnValue, new QTableWidgetItem("-") );
    _pLegendTable->item(row,cColummnValue)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    _pLegendTable->setItem(row, cColummnText, new QTableWidgetItem(_pGraphDataModel->label(graphIdx)));

    showGraph(graphIdx);
}

void Legend::toggleItemVisibility(qint32 activeGraphIdx)
{
    if (activeGraphIdx != -1)
    {
        const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(activeGraphIdx);

        _pGraphDataModel->setVisible(graphIdx, !_pGraphDataModel->isVisible(graphIdx));
    }
}

void Legend::showContextMenu(const QPoint& pos)
{
    const QPoint posInLegendContent = _pLegendTable->viewport()->mapFromParent(pos);
    qint32 row = _pLegendTable->indexAt(posInLegendContent).row();

    _popupMenuItem = row;

    if (row == -1)
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
        _pGraphDataModel->setVisible(idx, false);
    }
}

void Legend::showAll()
{
    for(qint32 idx = 0; idx < _pGraphDataModel->size(); idx++)
    {
        _pGraphDataModel->setVisible(idx, true);
    }
}
