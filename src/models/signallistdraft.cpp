#include "signallistdraft.h"

#include "models/graphdatamodel.h"
#include "util/expressionstatus.h"

#include <utility>

/*!
 * \brief Creates a draft holding a copy of every signal in \a pSource.
 * \param pSource         The real signal list; only apply() changes it.
 * \param pSettingsModel  Model with the configured devices, used to check the draft expressions.
 * \param parent          Parent object.
 */
SignalListDraft::SignalListDraft(GraphDataModel* pSource, SettingsModel* pSettingsModel, QObject* parent)
    : QObject(parent), _pSource(pSource), _pDraftModel(new GraphDataModel(this))
{
    for (qint32 idx = 0; idx < _pSource->size(); idx++)
    {
        appendCopy(_pDraftModel, _pSource, GraphIdx(idx));
    }
    resetOrigins();

    connect(_pDraftModel, &GraphDataModel::rowsInserted, this, &SignalListDraft::onRowsInserted);
    connect(_pDraftModel, &GraphDataModel::rowsRemoved, this, &SignalListDraft::onRowsRemoved);
    connect(_pDraftModel, &GraphDataModel::rowsMoved, this, &SignalListDraft::onRowsMoved);
    /* GraphDataModel never resets or relayouts; those would need resetting _origins as well */

    /* The copied rows carry the expression state of the source; this keeps it up to date for edits */
    _pExpressionStatus = new ExpressionStatus(_pDraftModel, pSettingsModel, this);
}

/*!
 * \brief Returns the model the dialog edits.
 * \return The draft model, owned by the draft.
 */
GraphDataModel* SignalListDraft::model()
{
    return _pDraftModel;
}

/*!
 * \brief Returns whether apply() would change the source.
 *
 * Compares the draft with the source, so an edit that is undone in the draft is no change.
 * \return True when signals were added, removed or reordered, or a setting differs from the source.
 */
bool SignalListDraft::hasChanges() const
{
    if (_pDraftModel->size() != _pSource->size())
    {
        return true;
    }

    for (qint32 row = 0; row < _pDraftModel->size(); row++)
    {
        if (_origins[row] != row)
        {
            return true;
        }

        if (!hasSameSettings(_pDraftModel, GraphIdx(row), _pSource, GraphIdx(row)))
        {
            return true;
        }
    }

    return false;
}

/*!
 * \brief Writes the draft back to the source.
 *
 * Signals that still exist keep their data, unless their expression changed or they were
 * deactivated: the source then clears it as for any other edit. Nothing is emitted when the draft
 * has no changes.
 */
void SignalListDraft::apply()
{
    if (!hasChanges())
    {
        return;
    }

    QList<int> sourceRows = removeDroppedSourceRows();

    for (qint32 row = 0; row < sourceRows.size(); row++)
    {
        if (sourceRows[row] != cNewRow)
        {
            copySettingsToSource(GraphIdx(row), GraphIdx(sourceRows[row]));
        }
    }

    for (qint32 row = 0; row < sourceRows.size(); row++)
    {
        if (sourceRows[row] == cNewRow)
        {
            appendCopy(_pSource, _pDraftModel, GraphIdx(row));
            sourceRows[row] = _pSource->size() - 1;
        }
    }

    moveSourceRowsIntoDraftOrder(sourceRows);
    resetOrigins();
}

void SignalListDraft::onRowsInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent);

    for (int row = first; row <= last; row++)
    {
        _origins.insert(row, cNewRow);
    }
}

void SignalListDraft::onRowsRemoved(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent);

    _origins.remove(first, last - first + 1);
}

void SignalListDraft::onRowsMoved(
  const QModelIndex& parent, int start, int end, const QModelIndex& destination, int row)
{
    Q_UNUSED(parent);
    Q_UNUSED(destination);

    const int count = end - start + 1;
    const QList<int> moved = _origins.mid(start, count);
    _origins.remove(start, count);

    /* row is a position in the list before the move, never inside the moved range */
    const int insertAt = row > end ? row - count : row;
    for (int idx = 0; idx < count; idx++)
    {
        _origins.insert(insertAt + idx, moved[idx]);
    }
}

/*!
 * \brief Appends a signal to \a pTarget with the settings of a signal in \a pFrom and no data.
 * \param pTarget  Model to append to.
 * \param pFrom    Model holding the signal to copy.
 * \param fromIdx  Row of the signal to copy.
 */
void SignalListDraft::appendCopy(GraphDataModel* pTarget, const GraphDataModel* pFrom, GraphIdx fromIdx)
{
    /* A new GraphData has its own data series; copying the source GraphData would share it */
    GraphData graphData;
    graphData.setLabel(pFrom->label(fromIdx));
    graphData.setColor(pFrom->color(fromIdx));
    graphData.setValueAxis(pFrom->valueAxis(fromIdx));
    graphData.setVisible(pFrom->isVisible(fromIdx));
    graphData.setActive(pFrom->isActive(fromIdx));
    graphData.setExpression(pFrom->expression(fromIdx));
    graphData.setExpressionState(pFrom->expressionState(fromIdx));
    pTarget->add(graphData);

    /* add() names a signal without a label; keep an empty label as it is */
    pTarget->setLabel(GraphIdx(pTarget->size() - 1), pFrom->label(fromIdx));
}

bool SignalListDraft::hasSameSettings(const GraphDataModel* pModel,
                                      GraphIdx idx,
                                      const GraphDataModel* pOther,
                                      GraphIdx otherIdx)
{
    return (pModel->label(idx) == pOther->label(otherIdx)) && (pModel->color(idx) == pOther->color(otherIdx)) &&
           (pModel->valueAxis(idx) == pOther->valueAxis(otherIdx)) &&
           (pModel->isVisible(idx) == pOther->isVisible(otherIdx)) &&
           (pModel->isActive(idx) == pOther->isActive(otherIdx)) &&
           (pModel->expression(idx) == pOther->expression(otherIdx));
}

/*!
 * \brief Copies the settings of a draft row to the source row it came from.
 *
 * The setters only emit for a real change. Visible is set after active, because activating a
 * signal makes it visible.
 * \param draftIdx   Row in the draft.
 * \param sourceIdx  Row in the source.
 */
void SignalListDraft::copySettingsToSource(GraphIdx draftIdx, GraphIdx sourceIdx)
{
    _pSource->setLabel(sourceIdx, _pDraftModel->label(draftIdx));
    _pSource->setColor(sourceIdx, _pDraftModel->color(draftIdx));
    _pSource->setValueAxis(sourceIdx, _pDraftModel->valueAxis(draftIdx));
    _pSource->setExpression(sourceIdx, _pDraftModel->expression(draftIdx));
    _pSource->setActive(sourceIdx, _pDraftModel->isActive(draftIdx));
    _pSource->setVisible(sourceIdx, _pDraftModel->isVisible(draftIdx));
}

/*!
 * \brief Removes the source rows that were removed in the draft.
 * \return For each draft row, the row in the source after the removal, or cNewRow for a row
 *         added in the draft.
 */
QList<int> SignalListDraft::removeDroppedSourceRows()
{
    QList<GraphIdx> droppedRows;
    for (qint32 row = 0; row < _pSource->size(); row++)
    {
        if (!_origins.contains(row))
        {
            droppedRows.append(GraphIdx(row));
        }
    }

    _pSource->removeSignals(droppedRows);

    QList<int> sourceRows;
    for (const int origin : std::as_const(_origins))
    {
        if (origin == cNewRow)
        {
            sourceRows.append(cNewRow);
            continue;
        }

        int droppedBefore = 0;
        for (const GraphIdx dropped : std::as_const(droppedRows))
        {
            if (dropped.v < origin)
            {
                droppedBefore++;
            }
        }
        sourceRows.append(origin - droppedBefore);
    }

    return sourceRows;
}

/*!
 * \brief Moves the source rows so they follow the draft order.
 * \param sourceRows For each draft row, its row in the source. The source holds exactly these rows.
 */
void SignalListDraft::moveSourceRowsIntoDraftOrder(const QList<int>& sourceRows)
{
    QList<int> draftRowAt(sourceRows.size());
    for (qint32 draftRow = 0; draftRow < sourceRows.size(); draftRow++)
    {
        draftRowAt[sourceRows[draftRow]] = draftRow;
    }

    for (qint32 target = 0; target < draftRowAt.size(); target++)
    {
        const qint32 current = draftRowAt.indexOf(target);
        if (current != target)
        {
            _pSource->moveSignal(GraphIdx(current), GraphIdx(target));
            draftRowAt.move(current, target);
        }
    }
}

void SignalListDraft::resetOrigins()
{
    _origins.clear();
    for (qint32 row = 0; row < _pDraftModel->size(); row++)
    {
        _origins.append(row);
    }
}
