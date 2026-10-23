#include "signaltableproxymodel.h"

#include "models/graphdatamodel.h"
#include "util/expressionreferences.h"
#include "util/util.h"

#include <QColor>

using State = DataPointDescription::State;
using ExpressionState = GraphData::ExpressionState;

namespace {

//! The adapter's reason for rejecting a data point, with a fallback when it gave none
QString invalidError(const DataPointDescription& description)
{
    return description.error.isEmpty() ? QStringLiteral("Not a valid data point") : description.error;
}

//! The description of a data point, or its reference text while there is none
QString shortText(const ExpressionReferences::Reference& reference, const DataPointDescription& description)
{
    if (description.state == State::Valid && !description.description.isEmpty())
    {
        return description.description;
    }

    return reference.text;
}

} // namespace

/*!
 * \brief Construct a proxy that shows \a pGraphDataModel with an extra Data point column.
 * \param pGraphDataModel   The signal list to show.
 * \param pDescriptionCache Cache that describes the data points of each signal.
 * \param parent            Optional QObject parent.
 */
SignalTableProxyModel::SignalTableProxyModel(GraphDataModel* pGraphDataModel,
                                             DataPointDescriptionCache* pDescriptionCache,
                                             QObject* parent)
    : QIdentityProxyModel(parent), _pGraphDataModel(pGraphDataModel), _pDescriptionCache(pDescriptionCache)
{
    setSourceModel(_pGraphDataModel);

    connect(_pGraphDataModel, &GraphDataModel::dataChanged, this, &SignalTableProxyModel::onSourceDataChanged);
    connect(_pDescriptionCache, &DataPointDescriptionCache::descriptionChanged, this,
            &SignalTableProxyModel::onDescriptionChanged);
}

/*!
 * \brief Returns the column that describes the data points, after the GraphDataModel columns.
 */
int SignalTableProxyModel::dataPointColumn() const
{
    return _pGraphDataModel->columnCount();
}

int SignalTableProxyModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return dataPointColumn() + 1;
}

QModelIndex SignalTableProxyModel::index(int row, int column, const QModelIndex& parent) const
{
    if (column != dataPointColumn())
    {
        return QIdentityProxyModel::index(row, column, parent);
    }

    if (parent.isValid() || row < 0 || row >= rowCount())
    {
        return QModelIndex();
    }

    return createIndex(row, column);
}

QModelIndex SignalTableProxyModel::parent(const QModelIndex& child) const
{
    if (child.column() == dataPointColumn())
    {
        return QModelIndex();
    }

    return QIdentityProxyModel::parent(child);
}

QModelIndex SignalTableProxyModel::sibling(int row, int column, const QModelIndex& idx) const
{
    return index(row, column, parent(idx));
}

QModelIndex SignalTableProxyModel::mapToSource(const QModelIndex& proxyIndex) const
{
    if (proxyIndex.column() == dataPointColumn())
    {
        return QModelIndex();
    }

    return QIdentityProxyModel::mapToSource(proxyIndex);
}

/*!
 * \brief Map a selection to the source, leaving out the Data point column it does not have.
 */
QItemSelection SignalTableProxyModel::mapSelectionToSource(const QItemSelection& selection) const
{
    const int lastSourceColumn = dataPointColumn() - 1;

    QItemSelection sourceColumns;
    for (const QItemSelectionRange& range : selection)
    {
        if (range.left() > lastSourceColumn)
        {
            continue;
        }

        const int right = qMin(range.right(), lastSourceColumn);
        sourceColumns.append(QItemSelectionRange(index(range.top(), range.left(), range.parent()),
                                                 index(range.bottom(), right, range.parent())));
    }

    return QIdentityProxyModel::mapSelectionToSource(sourceColumns);
}

QVariant SignalTableProxyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (index.column() == dataPointColumn())
    {
        return dataPointData(index.row(), role);
    }

    if (index.column() == GraphDataModel::column::EXPRESSION && role == Qt::ToolTipRole)
    {
        return expressionToolTip(index.row());
    }

    return QIdentityProxyModel::data(index, role);
}

QVariant SignalTableProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && section == dataPointColumn())
    {
        return role == Qt::DisplayRole ? QVariant(tr("Data point")) : QVariant();
    }

    return QIdentityProxyModel::headerData(section, orientation, role);
}

Qt::ItemFlags SignalTableProxyModel::flags(const QModelIndex& index) const
{
    if (index.isValid() && index.column() == dataPointColumn())
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
    }

    return QIdentityProxyModel::flags(index);
}

/*!
 * \brief Encode a drag of signal rows; the Data point column is left out, it has no source index.
 */
QMimeData* SignalTableProxyModel::mimeData(const QModelIndexList& indexes) const
{
    QModelIndexList sourceColumnIndexes;
    for (const QModelIndex& idx : indexes)
    {
        if (idx.column() != dataPointColumn())
        {
            sourceColumnIndexes.append(idx);
        }
    }

    /* GraphDataModel takes the dragged row from the first index */
    if (sourceColumnIndexes.isEmpty() && !indexes.isEmpty())
    {
        sourceColumnIndexes.append(index(indexes.first().row(), GraphDataModel::column::TEXT));
    }

    return QIdentityProxyModel::mimeData(sourceColumnIndexes);
}

/*!
 * \brief A change of a row's expression or its state also changes its Data point text.
 */
void SignalTableProxyModel::onSourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    if (!topLeft.isValid() || !bottomRight.isValid())
    {
        return;
    }

    emit dataChanged(index(topLeft.row(), dataPointColumn()), index(bottomRight.row(), dataPointColumn()));
}

/*!
 * \brief Update the rows that use the reference that was just described.
 */
void SignalTableProxyModel::onDescriptionChanged(const QString& referenceText)
{
    for (int row = 0; row < rowCount(); row++)
    {
        const auto references = ExpressionReferences::find(_pGraphDataModel->expression(GraphIdx(row)));
        for (const ExpressionReferences::Reference& reference : references)
        {
            if (reference.text == referenceText)
            {
                emit dataChanged(index(row, GraphDataModel::column::EXPRESSION), index(row, dataPointColumn()));
                break;
            }
        }
    }
}

/*!
 * \brief Build the Data point text of a row: the parser error, the description of its data point,
 * or the number of data points with a description of each in the tooltip.
 *
 * A data point without a description yet shows its reference text.
 */
SignalTableProxyModel::RowText SignalTableProxyModel::dataPointText(int row) const
{
    const GraphIdx graphIdx(row);
    RowText rowText;

    const ExpressionState state = _pGraphDataModel->expressionState(graphIdx);
    if (state == ExpressionState::SYNTAX_ERROR || state == ExpressionState::UNKNOWN_DEVICE)
    {
        const QModelIndex expressionIndex = _pGraphDataModel->index(row, GraphDataModel::column::EXPRESSION);
        rowText.toolTip = _pGraphDataModel->data(expressionIndex, Qt::ToolTipRole).toString();
        rowText.text = state == ExpressionState::SYNTAX_ERROR ? rowText.toolTip : tr("Unknown device");
        rowText.bError = true;
        return rowText;
    }

    const auto references = ExpressionReferences::find(_pGraphDataModel->expression(graphIdx));
    if (references.isEmpty())
    {
        return rowText;
    }

    QStringList lines;
    for (const ExpressionReferences::Reference& reference : references)
    {
        const DataPointDescription description = _pDescriptionCache->lookup(reference);
        if (description.state == State::Invalid)
        {
            rowText.bError = true;
            lines.append(QStringLiteral("%1: %2").arg(reference.text, invalidError(description)));
        }
        else
        {
            lines.append(shortText(reference, description));
        }
    }

    rowText.text = references.size() == 1 ? lines.first() : tr("%1 data points").arg(references.size());
    rowText.toolTip = lines.join('\n');

    /* A single data point shows its reference, its reason is in the tooltip */
    if (references.size() == 1 && rowText.bError)
    {
        rowText.text = references.first().text;
    }

    return rowText;
}

QVariant SignalTableProxyModel::dataPointData(int row, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::ToolTipRole && role != Qt::ForegroundRole)
    {
        return QVariant();
    }

    const RowText rowText = dataPointText(row);
    if (role == Qt::DisplayRole)
    {
        return rowText.text;
    }

    if (role == Qt::ToolTipRole)
    {
        return rowText.toolTip.isEmpty() ? QVariant() : QVariant(Util::plainTextToolTip(rowText.toolTip));
    }

    return rowText.bError ? QVariant(QColor(Qt::red)) : QVariant();
}

/*!
 * \brief The Expression tooltip, naming the data point the adapter rejects when the syntax is fine.
 */
QVariant SignalTableProxyModel::expressionToolTip(int row) const
{
    const QModelIndex sourceIndex = _pGraphDataModel->index(row, GraphDataModel::column::EXPRESSION);
    QString toolTip = _pGraphDataModel->data(sourceIndex, Qt::ToolTipRole).toString();
    if (_pGraphDataModel->isExpressionValid(GraphIdx(row)))
    {
        const QString error = invalidReferenceError(row);
        if (!error.isEmpty())
        {
            toolTip = tr("Invalid data point: %1").arg(error);
        }
    }

    /* The parser error quotes the expression, which can come from a project file */
    return toolTip.isEmpty() ? QVariant() : QVariant(Util::plainTextToolTip(toolTip));
}

/*!
 * \brief Returns the adapter's error for the first data point of a row it rejects, or an empty string.
 */
QString SignalTableProxyModel::invalidReferenceError(int row) const
{
    const auto references = ExpressionReferences::find(_pGraphDataModel->expression(GraphIdx(row)));
    for (const ExpressionReferences::Reference& reference : references)
    {
        const DataPointDescription description = _pDescriptionCache->lookup(reference);
        if (description.state == State::Invalid)
        {
            return invalidError(description);
        }
    }

    return QString();
}
