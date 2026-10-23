#ifndef SIGNALTABLEPROXYMODEL_H
#define SIGNALTABLEPROXYMODEL_H

#include "models/datapointdescriptioncache.h"

#include <QIdentityProxyModel>
#include <QItemSelection>

class GraphDataModel;

/*!
 * \brief The signal list as the Signals dialog table shows it: the GraphDataModel columns plus a
 * Data point column with the adapter's description of each signal's data points.
 *
 * The extra column only exists in this view; the descriptions depend on running adapters and are
 * read from a DataPointDescriptionCache.
 */
class SignalTableProxyModel : public QIdentityProxyModel
{
    Q_OBJECT

public:
    SignalTableProxyModel(GraphDataModel* pGraphDataModel,
                          DataPointDescriptionCache* pDescriptionCache,
                          QObject* parent = nullptr);

    int dataPointColumn() const;

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    QModelIndex sibling(int row, int column, const QModelIndex& idx) const override;
    QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;
    QItemSelection mapSelectionToSource(const QItemSelection& selection) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;

private slots:
    void onSourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void onDescriptionChanged(const QString& referenceText);

private:
    struct RowText
    {
        QString text;
        QString toolTip;
        bool bError{ false };
    };

    RowText dataPointText(int row) const;
    QVariant dataPointData(int row, int role) const;
    QVariant expressionToolTip(int row) const;
    QString invalidReferenceError(int row) const;

    GraphDataModel* _pGraphDataModel;
    DataPointDescriptionCache* _pDescriptionCache;
};

#endif // SIGNALTABLEPROXYMODEL_H
