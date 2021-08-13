
#include "graphdata.h"
#include "util.h"

#include "graphdatamodel.h"

GraphDataModel::GraphDataModel(SettingsModel * pSettingsModel, QObject *parent) : QAbstractTableModel(parent)
{
    _pSettingsModel = pSettingsModel;
    _graphData.clear();

    connect(this, &GraphDataModel::visibilityChanged, this, &GraphDataModel::modelDataChanged);
    connect(this, &GraphDataModel::labelChanged, this, &GraphDataModel::modelDataChanged);
    connect(this, &GraphDataModel::colorChanged, this, &GraphDataModel::modelDataChanged);
    connect(this, &GraphDataModel::activeChanged, this, &GraphDataModel::modelDataChanged);
    connect(this, &GraphDataModel::unsignedChanged, this, &GraphDataModel::modelDataChanged);
    connect(this, &GraphDataModel::bit32Changed, this, &GraphDataModel::modelDataChanged);
    connect(this, &GraphDataModel::expressionChanged, this, &GraphDataModel::modelDataChanged);
    connect(this, &GraphDataModel::registerAddressChanged, this, &GraphDataModel::modelDataChanged);
    connect(this, &GraphDataModel::connectionIdChanged, this, &GraphDataModel::modelDataChanged);

    /* When adding or removing graphs, the complete view should be refreshed to make sure all indexes are updated */
    connect(this, &GraphDataModel::added, this, &GraphDataModel::modelCompleteDataChanged);
    connect(this, &GraphDataModel::removed, this, &GraphDataModel::modelCompleteDataChanged);
}

int GraphDataModel::rowCount(const QModelIndex & /*parent*/) const
{
    return size();
}

int GraphDataModel::columnCount(const QModelIndex & /*parent*/) const
{
    return column::COUNT;
}

QVariant GraphDataModel::data(const QModelIndex &index, int role) const
{
    switch (index.column())
    {
    case column::COLOR:
        if (role == Qt::BackgroundColorRole)
        {
            return color(index.row());
        }
        break;
    case column::ACTIVE:
        if (role == Qt::CheckStateRole)
        {
            if (isActive(index.row()))
            {
                return Qt::Checked;
            }
            else
            {
                return Qt::Unchecked;
            }
        }
        break;
    case column::UNSIGNED:
        if (role == Qt::CheckStateRole)
        {
            if (isUnsigned(index.row()))
            {
                return Qt::Checked;
            }
            else
            {
                return Qt::Unchecked;
            }
        }
        break;
    case column::REGISTER:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return registerAddress(index.row());
        }
        break;
    case column::BIT32:
        if (role == Qt::CheckStateRole)
        {
            if (isBit32(index.row()))
            {
                return Qt::Checked;
            }
            else
            {
                return Qt::Unchecked;
            }
        }
        break;
    case column::TEXT:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return label(index.row());
        }
        break;
    case column::EXPRESSION:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return expression(index.row());
        }
        break;
    case column::CONNECTION_ID:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return QString("Connection %1").arg(connectionId(index.row()) + 1);
        }
        break;
    default:
        return QVariant();
        break;

    }

    return QVariant();
}

QVariant GraphDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
            case column::COLOR:
                return QString("Color");
            case column::ACTIVE:
                return QString("Active");
            case column::UNSIGNED:
                return QString("Unsigned");
            case column::REGISTER:
                return QString("Address");
            case column::BIT32:
                return QString("32 bit");
            case column::TEXT:
                return QString("Text");
            case column::EXPRESSION:
                return QString("Expression");
            case column::CONNECTION_ID:
                return QString("Connection");
            default:
                return QVariant();
            }
        }
        else
        {
            //Can't happen because it is hidden
        }
    }

    return QVariant();
}


bool GraphDataModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    bool bRet = true;

    switch (index.column())
    {
    case column::COLOR:
        if (role == Qt::EditRole)
        {
            if (value.canConvert(QMetaType::QColor))
            {
                QColor color = value.value<QColor>();
                setColor(index.row(), color);
            }
            else
            {
                bRet = false;
            }
        }
        break;
    case column::ACTIVE:
        if (role == Qt::CheckStateRole)
        {
            if (value == Qt::Checked)
            {
                setActive(index.row(), true);
            }
            else
            {
                setActive(index.row(), false);
            }
        }
        break;
    case column::UNSIGNED:
        if (role == Qt::CheckStateRole)
        {
            if (value == Qt::Checked)
            {
                setUnsigned(index.row(), true);
            }
            else
            {
                setUnsigned(index.row(), false);
            }
        }
        break;
    case column::REGISTER:
        if (role == Qt::EditRole)
        {
            bool bOk = false;

            if (value.canConvert(QMetaType::UInt))
            {
                 const quint32 newAddr = value.toUInt();
                 if (
                         (newAddr >= 40001)
                         && (newAddr <= 49999)
                    )
                 {
                     bOk = true;
                     setRegisterAddress(index.row(), (quint16)newAddr);
                 }
            }

            if (!bOk)
            {
                Util::showError(tr("Register address is not a valid address between 40001 and 49999."));
                bRet = false;
            }
        }
        break;
    case column::BIT32:
        if (role == Qt::CheckStateRole)
        {
            if (value == Qt::Checked)
            {
                setBit32(index.row(), true);
            }
            else
            {
                setBit32(index.row(), false);
            }
        }
        break;
    case column::TEXT:
        if (role == Qt::EditRole)
        {
            setLabel(index.row(), value.toString());
        }
        break;
    case column::EXPRESSION:
        if (role == Qt::EditRole)
        {
            QString newExpr = value.toString();

            if (newExpr.contains(QStringLiteral("val"), Qt::CaseInsensitive))
            {
                setExpression(index.row(), newExpr);
            }
            else
            {
                bRet = false;
                Util::showError(tr("Every expression should contain the \"VAL\" variable"));
                break;
            }
        }
        break;
    case column::CONNECTION_ID:
        if (role == Qt::EditRole)
        {
            bool bSuccess = false;
            const quint8 newConnectionId = static_cast<quint8>(value.toUInt(&bSuccess));

            if (
                    (bSuccess)
                    && (newConnectionId < SettingsModel::CONNECTION_ID_CNT)
                )
            {
                setConnectionId(index.row(), newConnectionId);
            }
            else
            {
                bRet = false;
                Util::showError(tr("Connection ID is not valid"));
                break;
            }
        }
        break;
    default:
        break;

    }

    // Notify view(s) of change
    emit dataChanged(index, index);

    return bRet;
}

Qt::ItemFlags GraphDataModel::flags(const QModelIndex & index) const
{
    Qt::ItemFlags itemFlags = Qt::NoItemFlags;

    /* default is enabled */
    itemFlags |= Qt::ItemIsEnabled;

    if (
            (index.column() == column::ACTIVE)
            || (index.column() == column::UNSIGNED)
            || (index.column() == column::BIT32)
        )
    {
        // checkable
        itemFlags |= Qt::ItemIsSelectable |  Qt::ItemIsUserCheckable;
    }
    else if (
             (index.column() == column::COLOR)
             || (index.column() == column::EXPRESSION)
             )
    {
        itemFlags |= Qt::ItemIsSelectable;
    }
    else
    {
        itemFlags |= Qt::ItemIsSelectable |  Qt::ItemIsEditable;
    }

    return itemFlags;
}


bool GraphDataModel::removeRows (int row, int count, const QModelIndex & parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(count);

    removeFromModel(row);

    return true;
}

bool GraphDataModel::insertRows (int row, int count, const QModelIndex &parent)
{
    if (
        (count != 1)
        || (row != size())
        )
    {
        qDebug() << "RegisterModel: Not supported";
    }

    Q_UNUSED(row);
    Q_UNUSED(count);
    Q_UNUSED(parent);

    add();

    return true;
}

qint32 GraphDataModel::size() const
{
    return _graphData.size();
}

qint32 GraphDataModel::activeCount() const
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

bool GraphDataModel::isBit32(quint32 index) const
{
    return _graphData[index].isBit32();
}

QString GraphDataModel::expression(quint32 index) const
{
    return _graphData[index].expression();
}

quint16 GraphDataModel::registerAddress(quint32 index) const
{
    return _graphData[index].registerAddress();
}

quint8 GraphDataModel::connectionId(quint8 index) const
{
    return _graphData[index].connectionId();
}

QSharedPointer<QCPGraphDataContainer> GraphDataModel::dataMap(quint32 index)
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

void GraphDataModel::setBit32(quint32 index, bool b32Bit)
{
    if (_graphData[index].isBit32() != b32Bit)
    {
         _graphData[index].setBit32(b32Bit);
         emit bit32Changed(index);
    }
}

void GraphDataModel::setExpression(quint32 index, QString expression)
{
    if (_graphData[index].expression() != expression)
    {
         _graphData[index].setExpression(expression);
         emit expressionChanged(index);
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

void GraphDataModel::setConnectionId(quint32 index, const quint8 &connectionId)
{
    if (_graphData[index].connectionId() != connectionId)
    {
         _graphData[index].setConnectionId(connectionId);
         emit connectionIdChanged(index);
    }
}

void GraphDataModel::add(GraphData rowData)
{
    addToModel(&rowData);
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
        removeFromModel(idx);
    }
}

void GraphDataModel::clear()
{
    if (_graphData.size() > 0)
    {
        beginRemoveRows(QModelIndex(), 0, _graphData.size() - 1);

        _graphData.clear();

        updateActiveGraphList();

        endRemoveRows();

        emit removed(0);
    }
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
    std::sort(pList->begin(), pList->end(), std::less<int>());
}

bool GraphDataModel::isPresent(quint16 addr, QString* pExpression)
{
    for (qint32 idx = 0; idx < _graphData.size(); idx++)
    {
        if (
            (_graphData[idx].registerAddress() == addr)
            && (_graphData[idx].expression() == pExpression)
        )
        {
            return true;
        }
    }

    return false;
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
    quint16 nextAddress = 40000;

    /* Find highest address of existing addresses */
    for (qint32 idx = 0; idx < _graphData.size(); idx++)
    {
        if (_graphData[idx].registerAddress() > nextAddress)
        {
            nextAddress = _graphData[idx].registerAddress();
        }
    }

    /* Set to next address */
    nextAddress++;

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

void GraphDataModel::modelDataChanged(quint32 idx)
{
    emit dataChanged(index(idx, 0), index(idx, columnCount() - 1));
}

void GraphDataModel::modelCompleteDataChanged()
{
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void GraphDataModel::addToModel(GraphData * pGraphData)
{
    /* Call function to prepare view */
    beginInsertRows(QModelIndex(), size(), size());

    /* Select color */
    if (!pGraphData->color().isValid())
    {
        quint32 colorIndex = _graphData.size() % Util::cColorlist.size();
        pGraphData->setColor(Util::cColorlist[colorIndex]);
    }

    _graphData.append(*pGraphData);

    updateActiveGraphList();

    /* Call function to trigger view update */
    endInsertRows();

    emit added(size() - 1);
}

void GraphDataModel::removeFromModel(qint32 row)
{
    beginRemoveRows(QModelIndex(), row, row);

    _graphData.removeAt(row);

    updateActiveGraphList();

    endRemoveRows();

    emit removed(row);
}
