

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

    _pLayout->setSpacing(4);
    _pLayout->setContentsMargins(1, 1, 1, 1); // This is redundant with setMargin, which is deprecated

    _pLayout->addWidget(_pNoGraphs);
    setLayout(_pLayout);
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

void Legend::mousePressEvent(QMouseEvent * event)
{
    Q_UNUSED(event);

    qint32 idx = itemUnderCursor();
    if (idx != -1)
    {
        _pGuiModel->setFrontGraph(idx);
    }
}

void Legend::mouseDoubleClickEvent(QMouseEvent * event)
{
    Q_UNUSED(event);

    qint32 idx = itemUnderCursor();
    if (idx != -1)
    {
        const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(idx);

        _pGraphDataModel->setVisible(graphIdx, !_pGraphDataModel->isVisible(graphIdx));
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
            _lastReceivedValueList.append(QString("(%1)").arg(Util::formatDoubleForExport(valueList[i])));
        }
        else
        {
            /* Show error */
            _lastReceivedValueList.append("(-)");
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


    if (_items.size() == legendDataValues.size())
    {
        for (qint32 i = 0; i < _items.size(); i++)
        {
            const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(i);
            _items[i]->setText(QString("%1 %2").arg(legendDataValues[i]).arg(_pGraphDataModel->label(graphIdx)));
        }
    }
}

void Legend::updateLegend()
{
    for(qint32 idx = 0; idx < _items.size(); idx++)
    {
        _pLayout->removeWidget(_items[idx]);
        delete _items[idx];
    }
    _items.clear();

    if (_pGraphDataModel->activeCount() != 0)
    {
        QList<quint16> activeList;

        _pGraphDataModel->activeGraphIndexList(&activeList);

        for (qint32 idx = 0; idx < activeList.size(); idx++)
        {
            addItem(activeList[idx]);
        }

        _pNoGraphs->setVisible(false);
    }
    else
    {
        _pNoGraphs->setVisible(true);
    }
}

void Legend::showGraph(quint32 graphIdx)
{
    const qint32 activeGraphIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

    if (activeGraphIdx != -1)
    {
        QFont font = _items[activeGraphIdx]->font();
        QPalette palette = _items[activeGraphIdx]->palette();

        if (_pGraphDataModel->isVisible(graphIdx))
        {
            font.setItalic(false);
            palette.setColor(QPalette::WindowText,Qt::black);
        }
        else
        {
            font.setItalic(true);
            palette.setColor(QPalette::WindowText,Qt::gray);
        }

        _items[activeGraphIdx]->setFont(font);
        _items[activeGraphIdx]->setPalette(palette);
    }
}

void Legend::changeGraphColor(const quint32 graphIdx)
{
    const qint32 activeGraphIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

    if (activeGraphIdx != -1)
    {
        _items[activeGraphIdx]->setColor(_pGraphDataModel->color(graphIdx));
    }
}

void Legend::changeGraphLabel(const quint32 graphIdx)
{
    const qint32 activeGraphIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

    if (activeGraphIdx != -1)
    {
       _items[activeGraphIdx]->setText(_pGraphDataModel->label(graphIdx));
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
            cursorValueList.append("[?]");
        }
    }
}

void Legend::addItem(quint32 graphIdx)
{
    LegendItem * pNewItem = new LegendItem(this);

    pNewItem->setText(_pGraphDataModel->label(graphIdx));
    pNewItem->setColor(_pGraphDataModel->color(graphIdx));

    _items.append(pNewItem);
    _pLayout->addWidget(pNewItem);
}

qint32 Legend::itemUnderCursor()
{
    qint32 widgetIdx = -1;

    for (qint32 idx = 0u; idx < _items.size(); idx++)
    {
        if (_items[idx]->underMouse())
        {
            widgetIdx = idx;
            break;
        }
    }

    return widgetIdx;
}

