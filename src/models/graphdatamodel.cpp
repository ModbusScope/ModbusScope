#include "graphdatamodel.h"
#include "graphdata.h"
#include "util.h"

GraphDataModel::GraphDataModel(QObject *parent) : QObject(parent)
{
    _graphData.clear();
}

qint32 GraphDataModel::size()
{
    return _graphData.size();
}

qint32 GraphDataModel::activeCount()
{
    return _activeGraphList.size();
}

bool GraphDataModel::isVisible(quint32 index) const
{
    return _graphData[index].isVisible();
}

QString GraphDataModel::label(quint32 index) const
{
    return _graphData[index].label();
}

QColor GraphDataModel::color(quint32 index) const
{
    return _graphData[index].color();
}

bool GraphDataModel::isActive(quint32 index) const
{
    return _graphData[index].isActive();
}

bool GraphDataModel::isUnsigned(quint32 index) const
{
    return _graphData[index].isUnsigned();
}

double GraphDataModel::multiplyFactor(quint32 index) const
{
    return _graphData[index].multiplyFactor();
}

double GraphDataModel::divideFactor(quint32 index) const
{
    return _graphData[index].divideFactor();
}

quint16 GraphDataModel::registerAddress(quint32 index) const
{
    return _graphData[index].registerAddress();
}

quint16 GraphDataModel::bitmask(quint32 index) const
{
    return _graphData[index].bitmask();
}

qint32 GraphDataModel::shift(quint32 index) const
{
    return _graphData[index].shift();
}

QCPDataMap * GraphDataModel::dataMap(quint32 index)
{
    return _graphData[index].dataMap();
}

void GraphDataModel::setVisible(quint32 index, bool bVisible)
{
    if (_graphData[index].isVisible() != bVisible)
    {
         _graphData[index].setVisible(bVisible);
         emit visibilityChanged(index);
    }
}

void GraphDataModel::setLabel(quint32 index, const QString &label)
{
    if (_graphData[index].label() != label)
    {
         _graphData[index].setLabel(label);
         emit labelChanged(index);
    }
}

void GraphDataModel::setColor(quint32 index, const QColor &color)
{
    if (_graphData[index].color() != color)
    {
         _graphData[index].setColor(color);
         emit colorChanged(index);
    }
}

void GraphDataModel::setActive(quint32 index, bool bActive)
{
    if (_graphData[index].isActive() != bActive)
    {
        _graphData[index].setActive(bActive);

        // Update activeGraphList
        updateActiveGraphList();

        // When deactivated, clear data
        if (!bActive)
        {
            _graphData[index].dataMap()->clear();
        }
        else
        {
            // when (re)-added, make sure graph is always visible
            _graphData[index].setVisible(true);
        }

        emit activeChanged(index);
    }
}

void GraphDataModel::setUnsigned(quint32 index, bool bUnsigned)
{
    if (_graphData[index].isUnsigned() != bUnsigned)
    {
         _graphData[index].setUnsigned(bUnsigned);
         emit unsignedChanged(index);
    }
}

void GraphDataModel::setMultiplyFactor(quint32 index, double multiplyFactor)
{
    if (_graphData[index].multiplyFactor() != multiplyFactor)
    {
         _graphData[index].setMultiplyFactor(multiplyFactor);
         emit multiplyFactorChanged(index);
    }
}

void GraphDataModel::setDivideFactor(quint32 index, double divideFactor)
{
    if (_graphData[index].divideFactor() != divideFactor)
    {
         _graphData[index].setDivideFactor(divideFactor);
         emit divideFactorChanged(index);
    }
}

void GraphDataModel::setRegisterAddress(quint32 index, const quint16 &registerAddress)
{
    if (_graphData[index].registerAddress() != registerAddress)
    {
         _graphData[index].setRegisterAddress(registerAddress);
         emit registerAddressChanged(index);
    }
}

void GraphDataModel::setBitmask(quint32 index, const quint16 &bitmask)
{
    if (_graphData[index].bitmask() != bitmask)
    {
         _graphData[index].setBitmask(bitmask);
         emit bitmaskChanged(index);
    }
}

void GraphDataModel::setShift(quint32 index, const qint32 &shift)
{
    if (_graphData[index].shift() != shift)
    {
         _graphData[index].setShift(shift);
         emit shiftChanged(index);
    }
}

void GraphDataModel::add(GraphData rowData)
{
    /* Select color */
    if (!rowData.color().isValid())
    {
        quint32 colorIndex = _graphData.size() % Util::cColorlist.size();
        rowData.setColor(Util::cColorlist[colorIndex]);
    }

    _graphData.append(rowData);

    if (rowData.isActive())
    {
        // Always make sure active event is send when active is true
        setActive(_graphData.size() - 1, false);
        setActive(_graphData.size() - 1, true);
    }
}

void GraphDataModel::add(QList<GraphData> graphDataList)
{
    for (qint32 idx = 0; idx < graphDataList.size(); idx++)
    {
        add(graphDataList[idx]);
    }
}

void GraphDataModel::add()
{
    GraphData data;

    data.setRegisterAddress(nextFreeAddress());
    data.setLabel(QString("Register %1").arg(data.registerAddress()));

    add(data);
}

void GraphDataModel::add(QList<QString> labelList, QList<double> timeData, QList<QList<double> > data)
{
    foreach(QString label, labelList)
    {
        add();
        setLabel(_graphData.size() - 1, label);
    }

    emit graphsAddData(timeData, data);
}

void GraphDataModel::removeRegister(qint32 idx)
{   
    if (idx < _graphData.size())
    {
        if (_graphData[idx].isActive())
        {
            setActive(idx, false);
        }
        _graphData.removeAt(idx);

        updateActiveGraphList();
    }

    emit removed(idx);
}

void GraphDataModel::clear()
{
    _graphData.clear();
    _activeGraphList.clear();

    emit cleared();
}

// Get sorted list of active (unique) register addresses
void GraphDataModel::activeGraphAddresList(QList<quint16> * pRegisterList)
{
    // Clear list
    pRegisterList->clear();

    foreach(quint32 idx, _activeGraphList)
    {
        if (!pRegisterList->contains(_graphData[idx].registerAddress()))
        {
            pRegisterList->append(_graphData[idx].registerAddress());
        }
    }

    // sort qList
    qSort(*pRegisterList);
}

// Get list of active graph indexes
void GraphDataModel::activeGraphIndexList(QList<quint16> * pList)
{
    // Clear list
    pList->clear();

    foreach(quint32 idx, _activeGraphList)
    {
        pList->append(idx);
    }

    // sort qList
    qSort(*pList);
}

bool GraphDataModel::areExclusive(quint16 * pRegister, quint16 * pBitmask)
{
    for (qint32 idx = 0; idx < (_graphData.size() - 1); idx++) // Don't need to check last entry
    {
        for (int checkIdx = (idx + 1); checkIdx < _graphData.size(); checkIdx++)
        {
            if (
                (_graphData[idx].registerAddress() == _graphData[checkIdx].registerAddress())
                && (_graphData[idx].bitmask() == _graphData[checkIdx].bitmask())
            )
            {
                *pRegister = _graphData[idx].registerAddress();
                *pBitmask = _graphData[idx].bitmask();
                return false;
            }
        }
    }

    return true;
}

qint32 GraphDataModel::convertToActiveGraphIndex(quint32 graphIdx)
{
    qint16 result = -1;
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

qint32 GraphDataModel::convertToGraphIndex(quint32 activeIdx)
{
    return _activeGraphList[activeIdx];
}

quint16 GraphDataModel::nextFreeAddress()
{
    // Create local copy of active register list
    quint16 nextAddress;
    QList<quint16> regList;
    activeGraphAddresList(&regList);

    if (regList.size() > 0)
    {
        nextAddress = regList.last() + 1;
    }
    else
    {
        nextAddress = 40001;
    }

    return nextAddress;
}

void GraphDataModel::updateActiveGraphList(void)
{
    // Clear list
    _activeGraphList.clear();

    for (qint32 idx = 0; idx < _graphData.size(); idx++)
    {
        if (_graphData[idx].isActive())
        {
            _activeGraphList.append(idx);
        }
    }
}
