#ifndef SIGNALLISTDRAFT_H
#define SIGNALLISTDRAFT_H

#include "util/graphindex.h"

#include <QList>
#include <QModelIndex>
#include <QObject>

class ExpressionStatus;
class GraphDataModel;
class SettingsModel;

/*!
 * \brief The signal list as the Signals dialog currently shows it.
 *
 * The dialog edits model() instead of the real signal list, so Cancel discards every change.
 * apply() writes the draft back. Draft rows have their own, empty data series: a row copied from
 * the source never shares samples with it, so deactivating a signal in the draft does not clear
 * the source data.
 *
 * A draft belongs to one dialog session. The source must outlive the draft and must not change
 * while the draft exists: the draft maps its rows to source rows by position.
 */
class SignalListDraft : public QObject
{
    Q_OBJECT

public:
    SignalListDraft(GraphDataModel* pSource, SettingsModel* pSettingsModel, QObject* parent = nullptr);

    GraphDataModel* model();
    bool hasChanges() const;
    void apply();

private slots:
    void onRowsInserted(const QModelIndex& parent, int first, int last);
    void onRowsRemoved(const QModelIndex& parent, int first, int last);
    void onRowsMoved(const QModelIndex& parent, int start, int end, const QModelIndex& destination, int row);

private:
    static constexpr int cNewRow = -1;

    static void appendCopy(GraphDataModel* pTarget, const GraphDataModel* pFrom, GraphIdx fromIdx);
    static bool hasSameSettings(const GraphDataModel* pModel,
                                GraphIdx idx,
                                const GraphDataModel* pOther,
                                GraphIdx otherIdx);
    void copySettingsToSource(GraphIdx draftIdx, GraphIdx sourceIdx);
    QList<int> removeDroppedSourceRows();
    void moveSourceRowsIntoDraftOrder(const QList<int>& sourceRows);
    void resetOrigins();

    GraphDataModel* _pSource;
    GraphDataModel* _pDraftModel;
    ExpressionStatus* _pExpressionStatus;

    //! Source row of each draft row, or cNewRow for a row added in the draft
    QList<int> _origins;
};

#endif // SIGNALLISTDRAFT_H
