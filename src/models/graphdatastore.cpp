
#include "graphdatastore.h"

#include "util/util.h"

#include <algorithm>

GraphDataStore::GraphDataStore(QObject* parent) : QObject(parent), _selectedGraphIdx(-1)
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

GraphData::valueAxis_t GraphDataStore::valueAxis(quint32 index) const
{
    return _graphData[index].valueAxis();
}

bool GraphDataStore::isVisible(quint32 index) const
{
    return _graphData[index].isVisible();
}

QString GraphDataStore::label(quint32 index) const
{
    return _graphData[index].label();
}

QColor GraphDataStore::color(quint32 index) const
{
    return _graphData[index].color();
}

bool GraphDataStore::isActive(quint32 index) const
{
    return _graphData[index].isActive();
}

QString GraphDataStore::expression(quint32 index) const
{
    return _graphData[index].expression();
}

GraphData::ExpressionState GraphDataStore::expressionState(quint32 index) const
{
    return _graphData[index].expressionState();
}

bool GraphDataStore::isExpressionValid(quint32 index) const
{
    return _graphData[index].isExpressionValid();
}

qint32 GraphDataStore::selectedGraph() const
{
    return _selectedGraphIdx;
}

QString GraphDataStore::simplifiedExpression(quint32 index) const
{
    return _graphData[index].expression().simplified();
}

QSharedPointer<const QCPGraphDataContainer> GraphDataStore::dataMap(quint32 index) const
{
    return _graphData[index].dataMap();
}

QSharedPointer<QCPGraphDataContainer> GraphDataStore::mutableDataMap(quint32 index)
{
    return _graphData[index].mutableDataMap();
}

void GraphDataStore::setValueAxis(quint32 index, GraphData::valueAxis_t axis)
{
    if (_graphData[index].valueAxis() != axis)
    {
        _graphData[index].setValueAxis(axis);
        emit valueAxisChanged(index);
    }
}

void GraphDataStore::setVisible(quint32 index, bool bVisible)
{
    if (_graphData[index].isVisible() != bVisible)
    {
        _graphData[index].setVisible(bVisible);
        emit visibilityChanged(index);

        if (!bVisible && (static_cast<qint32>(index) == _selectedGraphIdx))
        {
            setSelectedGraph(-1);
        }
    }
}

void GraphDataStore::setLabel(quint32 index, const QString& label)
{
    if (_graphData[index].label() != label)
    {
        _graphData[index].setLabel(label);
        emit labelChanged(index);
    }
}

void GraphDataStore::setColor(quint32 index, const QColor& color)
{
    if (_graphData[index].color() != color)
    {
        _graphData[index].setColor(color);
        emit colorChanged(index);
    }
}

void GraphDataStore::setActive(quint32 index, bool bActive)
{
    if (_graphData[index].isActive() != bActive)
    {
        _graphData[index].setActive(bActive);
        updateActiveGraphList();

        if (!bActive)
        {
            _graphData[index].mutableDataMap()->clear();
        }
        else
        {
            /* When re-activated, always make the graph visible */
            _graphData[index].setVisible(true);
        }

        emit activeChanged(index);
    }
}

void GraphDataStore::setExpression(quint32 index, QString expression)
{
    if (_graphData[index].expression() != expression)
    {
        _graphData[index].setExpression(expression);
        emit expressionChanged(index);
    }
}

void GraphDataStore::setExpressionState(quint32 index, GraphData::ExpressionState status)
{
    if (_graphData[index].expressionState() != status)
    {
        _graphData[index].setExpressionState(status);
        emit expressionStateChanged(index);
    }
}

void GraphDataStore::setSelectedGraph(qint32 index)
{
    if (index >= 0)
    {
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
void GraphDataStore::activeGraphIndexList(QList<quint16>& list) const
{
    list.clear();

    for (quint32 idx : _activeGraphList)
    {
        list.append(idx);
    }

    std::sort(list.begin(), list.end(), std::less<int>());
}

qint32 GraphDataStore::convertToActiveGraphIndex(quint32 graphIdx) const
{
    qint32 result = -1;
    for (qint32 activeIdx = 0; activeIdx < _activeGraphList.size(); activeIdx++)
    {
        if (_activeGraphList[activeIdx] == graphIdx)
        {
            result = activeIdx;
            break;
        }
    }
    return result;
}

qint32 GraphDataStore::convertToGraphIndex(quint32 activeIdx) const
{
    return _activeGraphList[activeIdx];
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

void GraphDataStore::eraseGraphDataAt(qint32 row)
{
    resetSelection();
    _graphData.removeAt(row);
    updateActiveGraphList();
}

void GraphDataStore::clearAllGraphData()
{
    resetSelection();
    _graphData.clear();
    updateActiveGraphList();
}

void GraphDataStore::moveGraphRow(int sourceRow, int destRow)
{
    resetSelection();
    _graphData.move(sourceRow, destRow);
    updateActiveGraphList();
}

void GraphDataStore::resetSelection()
{
    if (_selectedGraphIdx != -1)
    {
        _selectedGraphIdx = -1;
        emit selectedGraphChanged(-1);
    }
}

void GraphDataStore::updateActiveGraphList()
{
    _activeGraphList.clear();

    for (qint32 idx = 0; idx < _graphData.size(); idx++)
    {
        if (_graphData[idx].isActive())
        {
            _activeGraphList.append(idx);
        }
    }
}
