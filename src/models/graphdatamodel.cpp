
#include "graphdatamodel.h"

#include "customwidgets/centeredbox.h"
#include "models/communicationstatsmodel.h"
#include "models/graphdata.h"
#include "models/graphdatastore.h"
#include "util/util.h"

const QColor GraphDataModel::lightRed = QColor(240, 128, 128); // #f08080

GraphDataModel::GraphDataModel(QObject* parent)
    : QAbstractTableModel(parent),
      _defaultExpression(QStringLiteral("${h0}")),
      _pStore(new GraphDataStore(this)),
      _pStats(new CommunicationStatsModel(this))
{
    connect(_pStore, &GraphDataStore::valueAxisChanged, this, &GraphDataModel::valueAxisChanged);
    connect(_pStore, &GraphDataStore::visibilityChanged, this, &GraphDataModel::visibilityChanged);
    connect(_pStore, &GraphDataStore::labelChanged, this, &GraphDataModel::labelChanged);
    connect(_pStore, &GraphDataStore::colorChanged, this, &GraphDataModel::colorChanged);
    connect(_pStore, &GraphDataStore::activeChanged, this, &GraphDataModel::activeChanged);
    connect(_pStore, &GraphDataStore::expressionChanged, this, &GraphDataModel::expressionChanged);
    connect(_pStore, &GraphDataStore::expressionStateChanged, this, &GraphDataModel::expressionStateChanged);
    connect(_pStore, &GraphDataStore::selectedGraphChanged, this, &GraphDataModel::selectedGraphChanged);

    connect(this, &GraphDataModel::visibilityChanged, this, &GraphDataModel::modelDataChanged);
    connect(this, &GraphDataModel::labelChanged, this, &GraphDataModel::modelDataChanged);
    connect(this, &GraphDataModel::colorChanged, this, &GraphDataModel::modelDataChanged);
    connect(this, &GraphDataModel::activeChanged, this, &GraphDataModel::modelDataChanged);
    connect(this, &GraphDataModel::expressionChanged, this, &GraphDataModel::modelDataChanged);
    connect(this, &GraphDataModel::expressionStateChanged, this, &GraphDataModel::modelDataChanged);

    connect(_pStats, &CommunicationStatsModel::statsChanged, this, &GraphDataModel::communicationStatsChanged);
    connect(_pStats, &CommunicationStatsModel::timeStatsChanged, this, &GraphDataModel::communicationTimeStatsChanged);
}

int GraphDataModel::rowCount(const QModelIndex& /*parent*/) const
{
    return size();
}

int GraphDataModel::columnCount(const QModelIndex& /*parent*/) const
{
    return column::COUNT;
}

QVariant GraphDataModel::data(const QModelIndex& index, int role) const
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
        else if (role == CheckAlignmentRole)
        {
            return Qt::AlignCenter;
        }
        else
        {
            // nothing to do
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
        else if ((role == Qt::BackgroundRole) && !isExpressionValid(index.row()))
        {
            return lightRed;
        }
        else if (role == Qt::ToolTipRole)
        {
            if (expressionState(index.row()) == GraphData::ExpressionState::SYNTAX_ERROR)
            {
                return tr("The expression has a syntax error");
            }
            else if (expressionState(index.row()) == GraphData::ExpressionState::UNKNOWN_DEVICE)
            {
                return tr("The expression contains an unknown device");
            }
            else
            {
                return QString();
            }
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
                return QString("Name");
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
            // Can't happen because it is hidden
        }
    }

    return QVariant();
}

bool GraphDataModel::setData(const QModelIndex& index, const QVariant& value, int role)
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

            if ((bSuccess) && (newValueAxis < GraphData::VALUE_AXIS_CNT))
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

Qt::ItemFlags GraphDataModel::flags(const QModelIndex& index) const
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
        itemFlags |= Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    }
    else if (index.column() == column::COLOR)
    {
        /* Don't allow selection of color column (focus hides color indication) */
    }
    else
    {
        itemFlags |= Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }

    return itemFlags;
}

Qt::DropActions GraphDataModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

bool GraphDataModel::removeRows(int row, int count, const QModelIndex& parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(count);

    removeFromModel(row);

    return true;
}

bool GraphDataModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if ((count != 1) || (row != size()))
    {
        qDebug() << "RegisterModel: Not supported";
    }

    Q_UNUSED(row);
    Q_UNUSED(count);
    Q_UNUSED(parent);

    add();

    return true;
}

QMimeData* GraphDataModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* data = QAbstractTableModel::mimeData(indexes);

    if (data)
    {
        data->setData("row", QByteArray::number(indexes.at(0).row()));
    }

    return data;
}

bool GraphDataModel::dropMimeData(
  const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
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
    return _pStore->size();
}

qint32 GraphDataModel::activeCount() const
{
    return _pStore->activeCount();
}

GraphData::valueAxis_t GraphDataModel::valueAxis(quint32 index) const
{
    return _pStore->valueAxis(index);
}

bool GraphDataModel::isVisible(quint32 index) const
{
    return _pStore->isVisible(index);
}

QString GraphDataModel::label(quint32 index) const
{
    return _pStore->label(index);
}

QColor GraphDataModel::color(quint32 index) const
{
    return _pStore->color(index);
}

bool GraphDataModel::isActive(quint32 index) const
{
    return _pStore->isActive(index);
}

QString GraphDataModel::expression(quint32 index) const
{
    return _pStore->expression(index);
}

GraphData::ExpressionState GraphDataModel::expressionState(quint32 index) const
{
    return _pStore->expressionState(index);
}

bool GraphDataModel::isExpressionValid(quint32 index) const
{
    return _pStore->isExpressionValid(index);
}

qint32 GraphDataModel::selectedGraph() const
{
    return _pStore->selectedGraph();
}

QString GraphDataModel::simplifiedExpression(quint32 index) const
{
    return _pStore->simplifiedExpression(index);
}

QSharedPointer<const QCPGraphDataContainer> GraphDataModel::dataMap(quint32 index) const
{
    return _pStore->dataMap(index);
}

QSharedPointer<QCPGraphDataContainer> GraphDataModel::mutableDataMap(quint32 index)
{
    return _pStore->mutableDataMap(index);
}

qint64 GraphDataModel::communicationStartTime()
{
    return _pStats->startTime();
}

void GraphDataModel::setCommunicationStartTime(qint64 startTime)
{
    _pStats->setStartTime(startTime);
}

qint64 GraphDataModel::communicationEndTime()
{
    return _pStats->endTime();
}

void GraphDataModel::setCommunicationEndTime(qint64 endTime)
{
    _pStats->setEndTime(endTime);
}

void GraphDataModel::setCommunicationStats(quint32 successCount, quint32 errorCount)
{
    _pStats->setStats(successCount, errorCount);
}

void GraphDataModel::setMedianPollTime(quint32 pollTime)
{
    _pStats->setMedianPollTime(pollTime);
}

quint32 GraphDataModel::communicationErrorCount()
{
    return _pStats->errorCount();
}

quint32 GraphDataModel::communicationSuccessCount()
{
    return _pStats->successCount();
}

qint64 GraphDataModel::communicationRunTime()
{
    return _pStats->runTime();
}

quint32 GraphDataModel::medianPollTime()
{
    return _pStats->medianPollTime();
}

void GraphDataModel::setValueAxis(quint32 index, GraphData::valueAxis_t axis)
{
    _pStore->setValueAxis(index, axis);
}

void GraphDataModel::setVisible(quint32 index, bool bVisible)
{
    _pStore->setVisible(index, bVisible);
}

void GraphDataModel::setLabel(quint32 index, const QString& label)
{
    _pStore->setLabel(index, label);
}

void GraphDataModel::setColor(quint32 index, const QColor& color)
{
    _pStore->setColor(index, color);
}

void GraphDataModel::setActive(quint32 index, bool bActive)
{
    _pStore->setActive(index, bActive);
}

void GraphDataModel::setExpression(quint32 index, QString expression)
{
    _pStore->setExpression(index, expression);
}

void GraphDataModel::setExpressionState(quint32 index, GraphData::ExpressionState status)
{
    _pStore->setExpressionState(index, status);
}

void GraphDataModel::setSelectedGraph(qint32 index)
{
    _pStore->setSelectedGraph(index);
}

void GraphDataModel::add(GraphData rowData)
{
    addToModel(rowData);
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
    data.setLabel("New curve");
    data.setExpression(_defaultExpression);
    add(data);
}

/*!
 * \brief Sets the default expression used when adding a new register without an explicit expression.
 * \param expression The expression string to use as the default (e.g. \c ${h0}).
 */
void GraphDataModel::setDefaultExpression(const QString& expression)
{
    if (expression.isEmpty())
    {
        return;
    }
    _defaultExpression = expression;
}

void GraphDataModel::add(QList<QString> labelList)
{
    foreach (QString label, labelList)
    {
        add();
        setLabel(_pStore->size() - 1, label);
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
    if (idx < _pStore->size())
    {
        removeFromModel(idx);
    }
}

void GraphDataModel::clear()
{
    if (_pStore->size() > 0)
    {
        beginRemoveRows(QModelIndex(), 0, _pStore->size() - 1);
        _pStore->clearAllGraphData();
        endRemoveRows();
        emit removed(0);
    }
}

void GraphDataModel::activeGraphIndexList(QList<quint16>* pList)
{
    _pStore->activeGraphIndexList(pList);
}

qint32 GraphDataModel::convertToActiveGraphIndex(quint32 graphIdx)
{
    return _pStore->convertToActiveGraphIndex(graphIdx);
}

qint32 GraphDataModel::convertToGraphIndex(quint32 activeIdx)
{
    return _pStore->convertToGraphIndex(activeIdx);
}

void GraphDataModel::modelDataChanged(quint32 idx)
{
    emit dataChanged(index(idx, 0), index(idx, columnCount() - 1));
}

void GraphDataModel::modelCompleteDataChanged()
{
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void GraphDataModel::addToModel(GraphData graphData)
{
    beginInsertRows(QModelIndex(), _pStore->size(), _pStore->size());
    _pStore->insertGraphData(graphData);
    endInsertRows();
    emit added(_pStore->size() - 1);
    modelCompleteDataChanged();
}

void GraphDataModel::removeFromModel(qint32 row)
{
    beginRemoveRows(QModelIndex(), row, row);
    _pStore->eraseGraphDataAt(row);
    endRemoveRows();
    emit removed(row);
    modelCompleteDataChanged();
}

void GraphDataModel::moveRow(int sourceRow, int destRow)
{
    int newRow = destRow;
    if (newRow == -1 || newRow >= rowCount())
    {
        newRow = rowCount() - 1;
    }
    else if (newRow > sourceRow)
    {
        newRow--;
    }

    if (sourceRow != newRow)
    {
        _pStore->moveGraphRow(sourceRow, newRow);
    }

    modelCompleteDataChanged();
    emit moved();
}
