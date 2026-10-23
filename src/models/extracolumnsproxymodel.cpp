#include "extracolumnsproxymodel.h"

#include <utility>

/*!
 * \brief Construct a proxy that adds \a extraColumnCount columns after the source columns.
 * \param extraColumnCount Number of extra columns.
 * \param parent           Optional QObject parent.
 */
ExtraColumnsProxyModel::ExtraColumnsProxyModel(int extraColumnCount, QObject* parent)
    : QIdentityProxyModel(parent), _extraColumnCount(extraColumnCount)
{
    /* QIdentityProxyModel maps every persistent index through mapToSource(), which an extra column
       does not have; onSourceLayoutChanged() moves them by row instead */
    setHandleSourceLayoutChanges(false);
}

/*!
 * \brief Returns the proxy column of the first extra column, which is the number of source columns.
 */
int ExtraColumnsProxyModel::firstExtraColumn() const
{
    if (sourceModel() == nullptr)
    {
        return 0;
    }

    return sourceModel()->columnCount();
}

/*!
 * \brief Returns which extra column \a proxyColumn is (0 for the first), or -1 for a source column.
 */
int ExtraColumnsProxyModel::extraColumnIndex(int proxyColumn) const
{
    const int extraColumn = proxyColumn - firstExtraColumn();
    if (extraColumn < 0 || extraColumn >= _extraColumnCount)
    {
        return -1;
    }

    return extraColumn;
}

/*!
 * \brief Show \a pSourceModel and follow its layout changes.
 * \param pSourceModel The table model to show.
 */
void ExtraColumnsProxyModel::setSourceModel(QAbstractItemModel* pSourceModel)
{
    if (sourceModel() != nullptr)
    {
        disconnect(sourceModel(), &QAbstractItemModel::layoutAboutToBeChanged, this,
                   &ExtraColumnsProxyModel::onSourceLayoutAboutToBeChanged);
        disconnect(sourceModel(), &QAbstractItemModel::layoutChanged, this,
                   &ExtraColumnsProxyModel::onSourceLayoutChanged);
    }

    /* A layout change of the previous source can no longer finish */
    _layoutChangeProxyIndexes.clear();
    _layoutChangeSourceIndexes.clear();
    _layoutChangeColumns.clear();

    QIdentityProxyModel::setSourceModel(pSourceModel);

    if (pSourceModel != nullptr)
    {
        connect(pSourceModel, &QAbstractItemModel::layoutAboutToBeChanged, this,
                &ExtraColumnsProxyModel::onSourceLayoutAboutToBeChanged);
        connect(pSourceModel, &QAbstractItemModel::layoutChanged, this, &ExtraColumnsProxyModel::onSourceLayoutChanged);
    }
}

int ExtraColumnsProxyModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return firstExtraColumn() + _extraColumnCount;
}

int ExtraColumnsProxyModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return QIdentityProxyModel::rowCount(parent);
}

bool ExtraColumnsProxyModel::hasChildren(const QModelIndex& parent) const
{
    return rowCount(parent) > 0 && columnCount(parent) > 0;
}

QModelIndex ExtraColumnsProxyModel::index(int row, int column, const QModelIndex& parent) const
{
    if (extraColumnIndex(column) < 0)
    {
        return QIdentityProxyModel::index(row, column, parent);
    }

    if (parent.isValid() || row < 0 || row >= rowCount())
    {
        return QModelIndex();
    }

    return createIndex(row, column);
}

QModelIndex ExtraColumnsProxyModel::parent(const QModelIndex& child) const
{
    if (extraColumnIndex(child.column()) >= 0)
    {
        return QModelIndex();
    }

    return QIdentityProxyModel::parent(child);
}

QModelIndex ExtraColumnsProxyModel::sibling(int row, int column, const QModelIndex& idx) const
{
    return index(row, column, parent(idx));
}

QModelIndex ExtraColumnsProxyModel::mapToSource(const QModelIndex& proxyIndex) const
{
    if (extraColumnIndex(proxyIndex.column()) >= 0)
    {
        return QModelIndex();
    }

    return QIdentityProxyModel::mapToSource(proxyIndex);
}

/*!
 * \brief Map a selection to the source, leaving out the extra columns it does not have.
 */
QItemSelection ExtraColumnsProxyModel::mapSelectionToSource(const QItemSelection& selection) const
{
    const int lastSourceColumn = firstExtraColumn() - 1;

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

QVariant ExtraColumnsProxyModel::data(const QModelIndex& index, int role) const
{
    const int extraColumn = extraColumnIndex(index.column());
    if (index.isValid() && extraColumn >= 0)
    {
        return extraColumnData(index.row(), extraColumn, role);
    }

    return QIdentityProxyModel::data(index, role);
}

//! The extra columns are read-only.
bool ExtraColumnsProxyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (extraColumnIndex(index.column()) >= 0)
    {
        return false;
    }

    return QIdentityProxyModel::setData(index, value, role);
}

QVariant ExtraColumnsProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    const int extraColumn = extraColumnIndex(section);
    if (orientation == Qt::Horizontal && extraColumn >= 0)
    {
        return extraHeaderData(extraColumn, role);
    }

    return QIdentityProxyModel::headerData(section, orientation, role);
}

Qt::ItemFlags ExtraColumnsProxyModel::flags(const QModelIndex& index) const
{
    const int extraColumn = extraColumnIndex(index.column());
    if (index.isValid() && extraColumn >= 0)
    {
        return extraColumnFlags(index.row(), extraColumn);
    }

    return QIdentityProxyModel::flags(index);
}

/*!
 * \brief Encode a drag of rows; extra column indexes are left out, they have no source index.
 *
 * A drag that starts in an extra column is encoded as a drag of the first column of that row.
 */
QMimeData* ExtraColumnsProxyModel::mimeData(const QModelIndexList& indexes) const
{
    QModelIndexList sourceColumnIndexes;
    for (const QModelIndex& idx : indexes)
    {
        if (extraColumnIndex(idx.column()) < 0)
        {
            sourceColumnIndexes.append(idx);
        }
    }

    if (sourceColumnIndexes.isEmpty() && !indexes.isEmpty())
    {
        sourceColumnIndexes.append(index(indexes.first().row(), 0));
    }

    return QIdentityProxyModel::mimeData(sourceColumnIndexes);
}

/*!
 * \brief A drop on an extra column is passed to the source as a drop on its row, without a column.
 */
bool ExtraColumnsProxyModel::canDropMimeData(
  const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const
{
    const int sourceColumn = extraColumnIndex(column) >= 0 ? -1 : column;
    return QIdentityProxyModel::canDropMimeData(data, action, row, sourceColumn, parent);
}

/*!
 * \brief A drop on an extra column is passed to the source as a drop on its row, without a column.
 */
bool ExtraColumnsProxyModel::dropMimeData(
  const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    const int sourceColumn = extraColumnIndex(column) >= 0 ? -1 : column;
    return QIdentityProxyModel::dropMimeData(data, action, row, sourceColumn, parent);
}

/*!
 * \brief Returns the header of an extra column; empty by default.
 * \param extraColumn Which extra column (0 for the first).
 * \param role        The data role.
 */
QVariant ExtraColumnsProxyModel::extraHeaderData(int extraColumn, int role) const
{
    Q_UNUSED(extraColumn);
    Q_UNUSED(role);
    return QVariant();
}

/*!
 * \brief Returns the flags of a cell in an extra column: enabled, selectable and draggable by default.
 * \param row         The row.
 * \param extraColumn Which extra column (0 for the first).
 */
Qt::ItemFlags ExtraColumnsProxyModel::extraColumnFlags(int row, int extraColumn) const
{
    Q_UNUSED(row);
    Q_UNUSED(extraColumn);
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
}

/*!
 * \brief Notify views that the extra columns of rows \a firstRow to \a lastRow changed.
 */
void ExtraColumnsProxyModel::extraColumnsChanged(int firstRow, int lastRow)
{
    if (_extraColumnCount <= 0 || firstRow < 0 || lastRow >= rowCount() || firstRow > lastRow)
    {
        return;
    }

    const int lastColumn = firstExtraColumn() + _extraColumnCount - 1;
    emit dataChanged(index(firstRow, firstExtraColumn()), index(lastRow, lastColumn));
}

/*!
 * \brief Announce the layout change and remember the source row of every persistent index.
 */
void ExtraColumnsProxyModel::onSourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex>& sourceParents,
                                                            QAbstractItemModel::LayoutChangeHint hint)
{
    emit layoutAboutToBeChanged(proxyParents(sourceParents), hint);

    _layoutChangeProxyIndexes = persistentIndexList();
    _layoutChangeSourceIndexes.clear();
    _layoutChangeColumns.clear();
    for (const QModelIndex& proxyIndex : std::as_const(_layoutChangeProxyIndexes))
    {
        const int sourceColumn = extraColumnIndex(proxyIndex.column()) >= 0 ? 0 : proxyIndex.column();
        const QModelIndex sourceParent = QIdentityProxyModel::mapToSource(proxyIndex.parent());
        _layoutChangeSourceIndexes.append(sourceModel()->index(proxyIndex.row(), sourceColumn, sourceParent));
        _layoutChangeColumns.append(proxyIndex.column());
    }
}

/*!
 * \brief Move every persistent index to the new row of its source index, and finish the layout change.
 */
void ExtraColumnsProxyModel::onSourceLayoutChanged(const QList<QPersistentModelIndex>& sourceParents,
                                                   QAbstractItemModel::LayoutChangeHint hint)
{
    for (qsizetype idx = 0; idx < _layoutChangeProxyIndexes.size(); idx++)
    {
        const QPersistentModelIndex& sourceIndex = _layoutChangeSourceIndexes.at(idx);
        QModelIndex newProxyIndex;
        if (sourceIndex.isValid())
        {
            newProxyIndex = index(sourceIndex.row(), _layoutChangeColumns.at(idx), mapFromSource(sourceIndex.parent()));
        }
        changePersistentIndex(_layoutChangeProxyIndexes.at(idx), newProxyIndex);
    }

    _layoutChangeProxyIndexes.clear();
    _layoutChangeSourceIndexes.clear();
    _layoutChangeColumns.clear();

    emit layoutChanged(proxyParents(sourceParents), hint);
}

QList<QPersistentModelIndex> ExtraColumnsProxyModel::proxyParents(
  const QList<QPersistentModelIndex>& sourceParents) const
{
    QList<QPersistentModelIndex> parents;
    for (const QPersistentModelIndex& sourceParent : sourceParents)
    {
        parents.append(mapFromSource(sourceParent));
    }

    return parents;
}
