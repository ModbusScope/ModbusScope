
#include "graphdata.h"
#include "util.h"

#include "graphdatamodel.h"
#include "settingsmodel.h"

GraphDataModel::GraphDataModel(QObject *parent) : QAbstractTableModel(parent)
{
    _graphData.clear();

    connect(this, &GraphDataModel::visibilityChanged, this, &GraphDataModel::modelDataChanged);
    connect(this, &GraphDataModel::labelChanged, this, &GraphDataModel::modelDataChanged);
    connect(this, &GraphDataModel::colorChanged, this, &GraphDataModel::modelDataChanged);
    connect(this, &GraphDataModel::activeChanged, this, &GraphDataModel::modelDataChanged);
    connect(this, &GraphDataModel::expressionChanged, this, &GraphDataModel::modelDataChanged);

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
        if (role == Qt::BackgroundRole)
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
    case column::TEXT:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return label(index.row());
        }
        break;
    case column::EXPRESSION:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return simplifiedExpression(index.row());
        }
        break;
    case column::VALUE_AXIS:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            QString axis = valueAxis(index.row()) == GraphData::VALUE_AXIS_PRIMARY ? "Y1" : "Y2";
            return axis;
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
            case column::TEXT:
                return QString("Text");
            case column::EXPRESSION:
                return QString("Expression");
            case column::VALUE_AXIS:
                return QString("Y-Axis");
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
            if (value.canConvert<QColor>())
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
    case column::TEXT:
        if (role == Qt::EditRole)
        {
            setLabel(index.row(), value.toString());
        }
        break;
    case column::EXPRESSION:
        if (role == Qt::EditRole)
        {
            setExpression(index.row(), value.toString());
        }
        break;
    case column::VALUE_AXIS:
        if (role == Qt::EditRole)
        {
            bool bSuccess = false;
            const quint8 newValueAxis = static_cast<quint8>(value.toUInt(&bSuccess));

            if (
                    (bSuccess)
                    && (newValueAxis < GraphData::VALUE_AXIS_CNT)
                )
            {
                setValueAxis(index.row(), static_cast<GraphData::valueAxis_t>(newValueAxis));
            }
            else
            {
                bRet = false;
                Util::showError(tr("Y axis setting is not valid"));
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

    itemFlags |= Qt::ItemIsDragEnabled;

    /* Only allow drop on not existing items (eg between items) */
    if (!index.isValid())
    {
        itemFlags |= Qt::ItemIsDropEnabled;
    }

    /* default is enabled */
    itemFlags |= Qt::ItemIsEnabled;

    if (index.column() == column::ACTIVE)
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

Qt::DropActions GraphDataModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

bool GraphDataModel::removeRows(int row, int count, const QModelIndex & parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(count);

    removeFromModel(row);

    return true;
}

bool GraphDataModel::insertRows(int row, int count, const QModelIndex &parent)
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

QMimeData *GraphDataModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *data = QAbstractTableModel::mimeData(indexes);

    if (data)
    {
        data->setData("row", QByteArray::number(indexes.at(0).row()));
    }

    return data;
}

bool GraphDataModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(column);

    if (!data || action != Qt::MoveAction)
    {
        return false;
    }

    bool bOk = false;
    const int old_row = data->data("row").toInt(&bOk);
    if (!bOk)
    {
        return false;
    }

    moveRow(old_row, row);

    return false;
}

qint32 GraphDataModel::size() const
{
    return _graphData.size();
}

qint32 GraphDataModel::activeCount() const
{
    return _activeGraphList.size();
}

GraphData::valueAxis_t GraphDataModel::valueAxis(quint32 index) const
{
    return _graphData[index].valueAxis();
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

QString GraphDataModel::expression(quint32 index) const
{
    return _graphData[index].expression();
}

QString GraphDataModel::simplifiedExpression(quint32 index) const
{
    return _graphData[index].expression().simplified();
}

QSharedPointer<QCPGraphDataContainer> GraphDataModel::dataMap(quint32 index)
{
    return _graphData[index].dataMap();
}

void GraphDataModel::setValueAxis(quint32 index, GraphData::valueAxis_t axis)
{
    if (_graphData[index].valueAxis() != axis)
    {
         _graphData[index].setValueAxis(axis);
         emit valueAxisChanged(index);
    }
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

void GraphDataModel::setExpression(quint32 index, QString expression)
{
    if (_graphData[index].expression() != expression)
    {
         _graphData[index].setExpression(expression);
         emit expressionChanged(index);
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

    quint32 registerAddr = 40001;
    data.setLabel(QString("Register %1").arg(registerAddr));
    data.setExpression(QString("${%1}").arg(registerAddr));

    add(data);
}

void GraphDataModel::add(QList<QString> labelList)
{
    foreach(QString label, labelList)
    {
        add();
        setLabel(_graphData.size() - 1, label);
    }
}

void GraphDataModel::setAllData(QList<double> timeData, QList<QList<double> > data)
{
    if (data.size() == size())
    {
        emit graphsAddData(timeData, data);
    }
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

void GraphDataModel::moveRow(int sourceRow, int destRow)
{
    int newRow = destRow;
    if(newRow == -1 || newRow >= rowCount())
    {
        newRow = rowCount() - 1;
    }
    else if (newRow > sourceRow)
    {
        newRow--;
    }

    if (sourceRow != newRow)
    {
        _graphData.move(sourceRow, newRow);
    }

    modelCompleteDataChanged();

    emit moved();
}
