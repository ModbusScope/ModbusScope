
#include "graphdatamodel.h"

#include "customwidgets/centeredbox.h"
#include "models/graphdata.h"
#include "models/graphdatastore.h"
#include "util/util.h"

#include <QMimeData>

const QColor GraphDataModel::lightRed = QColor(240, 128, 128); // #f08080

GraphDataModel::GraphDataModel(QObject* parent)
    : QAbstractTableModel(parent), _defaultExpression(QStringLiteral("${h0}")), _pStore(new GraphDataStore(this))
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
    if (!index.isValid() || index.row() < 0 || index.row() >= _pStore->size())
    {
        return QVariant();
    }

    const GraphIdx graphIdx(index.row());

    switch (index.column())
    {
    case column::COLOR:
        if (role == Qt::BackgroundRole)
        {
            return color(graphIdx);
        }
        break;
    case column::ACTIVE:
        if (role == Qt::CheckStateRole)
        {
            if (isActive(graphIdx))
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
            return label(graphIdx);
        }
        break;
    case column::EXPRESSION:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return simplifiedExpression(graphIdx);
        }
        else if ((role == Qt::BackgroundRole) && !isExpressionValid(graphIdx))
        {
            return lightRed;
        }
        else if (role == Qt::ToolTipRole)
        {
            if (expressionState(graphIdx) == GraphData::ExpressionState::SYNTAX_ERROR)
            {
                return tr("The expression has a syntax error");
            }
            else if (expressionState(graphIdx) == GraphData::ExpressionState::UNKNOWN_DEVICE)
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
            QString axis = valueAxis(graphIdx) == GraphData::VALUE_AXIS_PRIMARY ? "Y1" : "Y2";
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
    if (!index.isValid() || index.row() < 0 || index.row() >= _pStore->size())
    {
        return false;
    }

    bool bRet = true;
    const GraphIdx graphIdx(index.row());

    switch (index.column())
    {
    case column::COLOR:
        if (role == Qt::EditRole)
        {
            if (value.canConvert<QColor>())
            {
                QColor color = value.value<QColor>();
                setColor(graphIdx, color);
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
                setActive(graphIdx, true);
            }
            else
            {
                setActive(graphIdx, false);
            }
        }
        break;
    case column::TEXT:
        if (role == Qt::EditRole)
        {
            setLabel(graphIdx, value.toString());
        }
        break;
    case column::EXPRESSION:
        if (role == Qt::EditRole)
        {
            setExpression(graphIdx, value.toString());
        }
        break;
    case column::VALUE_AXIS:
        if (role == Qt::EditRole)
        {
            bool bSuccess = false;
            const quint8 newValueAxis = static_cast<quint8>(value.toUInt(&bSuccess));

            if ((bSuccess) && (newValueAxis < GraphData::VALUE_AXIS_CNT))
            {
                setValueAxis(graphIdx, static_cast<GraphData::valueAxis_t>(newValueAxis));
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

/*!
 * \brief Returns the value axis assignment for the given graph.
 * \param index GraphIdx identifying the graph in the full (active + inactive) list.
 * \return Primary or secondary axis assignment; default-constructed value if index is invalid.
 */
GraphData::valueAxis_t GraphDataModel::valueAxis(GraphIdx index) const
{
    return _pStore->valueAxis(index);
}

/*!
 * \brief Returns whether the graph is currently visible in the plot.
 * \param index GraphIdx identifying the graph.
 * \return True if visible; false if hidden or if \a index is invalid.
 */
bool GraphDataModel::isVisible(GraphIdx index) const
{
    return _pStore->isVisible(index);
}

/*!
 * \brief Returns the display label for the given graph.
 * \param index GraphIdx identifying the graph.
 * \return Label string; empty string if \a index is invalid.
 */
QString GraphDataModel::label(GraphIdx index) const
{
    return _pStore->label(index);
}

QColor GraphDataModel::color(GraphIdx index) const
{
    return _pStore->color(index);
}

bool GraphDataModel::isActive(GraphIdx index) const
{
    return _pStore->isActive(index);
}

QString GraphDataModel::expression(GraphIdx index) const
{
    return _pStore->expression(index);
}

GraphData::ExpressionState GraphDataModel::expressionState(GraphIdx index) const
{
    return _pStore->expressionState(index);
}

bool GraphDataModel::isExpressionValid(GraphIdx index) const
{
    return _pStore->isExpressionValid(index);
}

GraphIdx GraphDataModel::selectedGraph() const
{
    return _pStore->selectedGraph();
}

QString GraphDataModel::simplifiedExpression(GraphIdx index) const
{
    return _pStore->simplifiedExpression(index);
}

/*!
 * \brief Returns a read-only view of the graph's time-series data.
 * \param index GraphIdx identifying the graph.
 * \return Shared pointer to the data series; null if \a index is invalid.
 */
QSharedPointer<const GraphDataSeries> GraphDataModel::dataSeries(GraphIdx index) const
{
    return _pStore->dataSeries(index);
}

/*!
 * \brief Returns a writable reference to the graph's time-series data.
 * \param index GraphIdx identifying the graph.
 * \return Shared pointer to the data series; null if \a index is invalid.
 */
QSharedPointer<GraphDataSeries> GraphDataModel::mutableDataSeries(GraphIdx index)
{
    return _pStore->mutableDataSeries(index);
}

void GraphDataModel::setValueAxis(GraphIdx index, GraphData::valueAxis_t axis)
{
    _pStore->setValueAxis(index, axis);
}

void GraphDataModel::setVisible(GraphIdx index, bool bVisible)
{
    _pStore->setVisible(index, bVisible);
}

void GraphDataModel::setLabel(GraphIdx index, const QString& label)
{
    _pStore->setLabel(index, label);
}

void GraphDataModel::setColor(GraphIdx index, const QColor& color)
{
    _pStore->setColor(index, color);
}

void GraphDataModel::setActive(GraphIdx index, bool bActive)
{
    _pStore->setActive(index, bActive);
}

void GraphDataModel::setExpression(GraphIdx index, QString expression)
{
    _pStore->setExpression(index, expression);
}

void GraphDataModel::setExpressionState(GraphIdx index, GraphData::ExpressionState status)
{
    _pStore->setExpressionState(index, status);
}

void GraphDataModel::setSelectedGraph(GraphIdx index)
{
    _pStore->setSelectedGraph(index);
}

/*!
 * \brief Appends a single graph with the given data to the model.
 * \param rowData Fully populated GraphData to add.
 */
void GraphDataModel::add(GraphData rowData)
{
    addToModel(rowData);
}

/*!
 * \brief Appends a list of graphs to the model.
 * \param graphDataList List of GraphData entries to add in order.
 */
void GraphDataModel::add(QList<GraphData> graphDataList)
{
    for (qint32 idx = 0; idx < graphDataList.size(); idx++)
    {
        add(graphDataList[idx]);
    }
}

/*!
 * \brief Appends a new empty graph using the default expression.
 */
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
        setLabel(GraphIdx(_pStore->size() - 1), label);
    }
}

void GraphDataModel::setAllData(QList<double> timeData, QList<QList<double> > data)
{
    if (data.size() == size())
    {
        emit graphsAddData(timeData, data);
    }
}

void GraphDataModel::removeRegister(GraphIdx idx)
{
    if (idx.isValid() && idx.v < _pStore->size())
    {
        removeFromModel(idx.v);
    }
}

void GraphDataModel::clear()
{
    if (_pStore->size() > 0)
    {
        beginRemoveRows(QModelIndex(), 0, _pStore->size() - 1);
        _pStore->clearAllGraphData();
        endRemoveRows();
        emit removed(GraphIdx(0));
    }
}

/*!
 * \brief Fills \a list with the GraphIdx of every currently active graph, in ascending order.
 * \param list Output list; existing contents are replaced.
 */
void GraphDataModel::activeGraphIndexList(QList<GraphIdx>& list) const
{
    _pStore->activeGraphIndexList(list);
}

/*!
 * \brief Converts a GraphIdx to the corresponding ActiveIdx.
 *
 * A GraphIdx identifies a graph's position in the full (active + inactive) list.
 * An ActiveIdx is its sequential slot in QCustomPlot (active graphs only, 0-based).
 *
 * \param graphIdx Graph to look up.
 * \return The ActiveIdx for the graph, or an invalid ActiveIdx (isValid() == false)
 *         if the graph is not currently active.
 */
ActiveIdx GraphDataModel::convertToActiveGraphIndex(GraphIdx graphIdx) const
{
    return _pStore->convertToActiveGraphIndex(graphIdx);
}

/*!
 * \brief Converts an ActiveIdx back to the corresponding GraphIdx.
 * \param activeIdx Active-graph slot to look up.
 * \return The GraphIdx for the slot, or an invalid GraphIdx (isValid() == false)
 *         if \a activeIdx is out of range.
 */
GraphIdx GraphDataModel::convertToGraphIndex(ActiveIdx activeIdx) const
{
    return _pStore->convertToGraphIndex(activeIdx);
}

void GraphDataModel::modelDataChanged(GraphIdx idx)
{
    emit dataChanged(index(idx.v, 0), index(idx.v, columnCount() - 1));
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
    emit added(GraphIdx(_pStore->size() - 1));
    modelCompleteDataChanged();
}

void GraphDataModel::removeFromModel(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    _pStore->eraseGraphDataAt(GraphIdx(row));
    endRemoveRows();
    emit removed(GraphIdx(row));
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
        const int destinationChild = newRow > sourceRow ? newRow + 1 : newRow;
        beginMoveRows(QModelIndex(), sourceRow, sourceRow, QModelIndex(), destinationChild);
        _pStore->moveGraphRow(GraphIdx(sourceRow), GraphIdx(newRow));
        endMoveRows();
    }

    modelCompleteDataChanged();
    emit moved();
}
