
#include "graphdatastore.h"

#include "util/util.h"

#include <algorithm>

GraphDataStore::GraphDataStore(QObject* parent) : QObject(parent)
{
}

qint32 GraphDataStore::size() const
{
    return _graphData.size();
}

qint32 GraphDataStore::activeCount() const
{
    return _activeGraphList.size();
}

GraphData::valueAxis_t GraphDataStore::valueAxis(GraphIdx index) const
{
    return _graphData[index.v].valueAxis();
}

bool GraphDataStore::isVisible(GraphIdx index) const
{
    return _graphData[index.v].isVisible();
}

QString GraphDataStore::label(GraphIdx index) const
{
    return _graphData[index.v].label();
}

QColor GraphDataStore::color(GraphIdx index) const
{
    return _graphData[index.v].color();
}

bool GraphDataStore::isActive(GraphIdx index) const
{
    return _graphData[index.v].isActive();
}

QString GraphDataStore::expression(GraphIdx index) const
{
    return _graphData[index.v].expression();
}

GraphData::ExpressionState GraphDataStore::expressionState(GraphIdx index) const
{
    return _graphData[index.v].expressionState();
}

bool GraphDataStore::isExpressionValid(GraphIdx index) const
{
    return _graphData[index.v].isExpressionValid();
}

GraphIdx GraphDataStore::selectedGraph() const
{
    return _selectedGraphIdx;
}

QString GraphDataStore::simplifiedExpression(GraphIdx index) const
{
    return _graphData[index.v].expression().simplified();
}

QSharedPointer<const QCPGraphDataContainer> GraphDataStore::dataMap(GraphIdx index) const
{
    return _graphData[index.v].dataMap();
}

QSharedPointer<QCPGraphDataContainer> GraphDataStore::mutableDataMap(GraphIdx index)
{
    return _graphData[index.v].mutableDataMap();
}

void GraphDataStore::setValueAxis(GraphIdx index, GraphData::valueAxis_t axis)
{
    if (_graphData[index.v].valueAxis() != axis)
    {
        _graphData[index.v].setValueAxis(axis);
        emit valueAxisChanged(index);
    }
}

void GraphDataStore::setVisible(GraphIdx index, bool bVisible)
{
    if (_graphData[index.v].isVisible() != bVisible)
    {
        _graphData[index.v].setVisible(bVisible);
        emit visibilityChanged(index);

        if (!bVisible && (index == _selectedGraphIdx))
        {
            setSelectedGraph(GraphIdx());
        }
    }
}

void GraphDataStore::setLabel(GraphIdx index, const QString& label)
{
    if (_graphData[index.v].label() != label)
    {
        _graphData[index.v].setLabel(label);
        emit labelChanged(index);
    }
}

void GraphDataStore::setColor(GraphIdx index, const QColor& color)
{
    if (_graphData[index.v].color() != color)
    {
        _graphData[index.v].setColor(color);
        emit colorChanged(index);
    }
}

void GraphDataStore::setActive(GraphIdx index, bool bActive)
{
    if (_graphData[index.v].isActive() != bActive)
    {
        _graphData[index.v].setActive(bActive);
        updateActiveGraphList();

        if (!bActive)
        {
            _graphData[index.v].mutableDataMap()->clear();
        }
        else
        {
            /* When re-activated, always make the graph visible */
            setVisible(index, true);
        }

        emit activeChanged(index);
    }
}

void GraphDataStore::setExpression(GraphIdx index, QString expression)
{
    if (_graphData[index.v].expression() != expression)
    {
        _graphData[index.v].setExpression(expression);
        emit expressionChanged(index);
    }
}

void GraphDataStore::setExpressionState(GraphIdx index, GraphData::ExpressionState status)
{
    if (_graphData[index.v].expressionState() != status)
    {
        _graphData[index.v].setExpressionState(status);
        emit expressionStateChanged(index);
    }
}

void GraphDataStore::setSelectedGraph(GraphIdx index)
{
    if (index.isValid())
    {
        if (index.v >= _graphData.size())
        {
            return;
        }

        if (!isVisible(index))
        {
            return;
        }
    }

    if (index != _selectedGraphIdx)
    {
        _selectedGraphIdx = index;
        emit selectedGraphChanged(_selectedGraphIdx);
    }
}

/*!
 * \brief Returns a sorted list of the indices of all active graphs.
 */
void GraphDataStore::activeGraphIndexList(QList<GraphIdx>& list) const
{
    list.clear();

    for (GraphIdx idx : _activeGraphList)
    {
        list.append(idx);
    }

    std::sort(list.begin(), list.end());
}

ActiveIdx GraphDataStore::convertToActiveGraphIndex(GraphIdx graphIdx) const
{
    for (qint32 activeIdx = 0; activeIdx < _activeGraphList.size(); activeIdx++)
    {
        if (_activeGraphList[activeIdx] == graphIdx)
        {
            return ActiveIdx(activeIdx);
        }
    }
    return ActiveIdx();
}

GraphIdx GraphDataStore::convertToGraphIndex(ActiveIdx activeIdx) const
{
    if (!activeIdx.isValid() || activeIdx.v >= _activeGraphList.size())
    {
        return GraphIdx();
    }
    return _activeGraphList[activeIdx.v];
}

/*!
 * \brief Appends \a graphData to the store, assigning a color if none is set.
 */
void GraphDataStore::insertGraphData(GraphData graphData)
{
    if (!graphData.color().isValid())
    {
        quint32 colorIndex = _graphData.size() % Util::cColorlist.size();
        graphData.setColor(Util::cColorlist[colorIndex]);
    }

    _graphData.append(graphData);
    updateActiveGraphList();
}

void GraphDataStore::eraseGraphDataAt(GraphIdx row)
{
    resetSelection();
    _graphData.removeAt(row.v);
    updateActiveGraphList();
}

void GraphDataStore::clearAllGraphData()
{
    resetSelection();
    _graphData.clear();
    updateActiveGraphList();
}

void GraphDataStore::moveGraphRow(GraphIdx sourceRow, GraphIdx destRow)
{
    resetSelection();
    _graphData.move(sourceRow.v, destRow.v);
    updateActiveGraphList();
}

void GraphDataStore::resetSelection()
{
    if (_selectedGraphIdx.isValid())
    {
        _selectedGraphIdx = GraphIdx();
        emit selectedGraphChanged(GraphIdx());
    }
}

void GraphDataStore::updateActiveGraphList()
{
    _activeGraphList.clear();

    for (qint32 idx = 0; idx < _graphData.size(); idx++)
    {
        if (_graphData[idx].isActive())
        {
            _activeGraphList.append(GraphIdx(idx));
        }
    }
}
