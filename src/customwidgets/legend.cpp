

#include <QStyleOption>
#include <QDebug>
#include <QPainter>
#include <QMouseEvent>

#include "guimodel.h"
#include "graphdatamodel.h"

#include "legend.h"

Legend::Legend(QWidget *parent) : QFrame(parent)
{
    _pLayout = new QVBoxLayout();

    _pNoGraphs = new QLabel("No active graphs");

    _pLayout->setSpacing(0);
    _pLayout->setContentsMargins(0,0,0,0); // This is redundant with setMargin, which is deprecated

    _pLayout->addWidget(_pNoGraphs);
    setLayout(_pLayout);
}

void Legend::setModels(GuiModel *pGuiModel, GraphDataModel * pGraphDataModel)
{
    _pGuiModel = pGuiModel;
    _pGraphDataModel = pGraphDataModel;

    connect(_pGraphDataModel, SIGNAL(activeChanged(quint32)), this, SLOT(updateLegend()));
}

void Legend::mousePressEvent(QMouseEvent * event)
{
    Q_UNUSED(event);

    qint32 idx = itemUnderCursor();
    if (idx != -1)
    {
        qDebug() << "clicked: " << idx;
    }
}

void Legend::mouseDoubleClickEvent(QMouseEvent * event)
{
    Q_UNUSED(event);

    qint32 idx = itemUnderCursor();
    if (idx != -1)
    {
        qDebug() << "double clicked: " << idx;
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

