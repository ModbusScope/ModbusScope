#ifndef SIGNALTABLEPROXYMODEL_H
#define SIGNALTABLEPROXYMODEL_H

#include "models/datapointdescriptioncache.h"
#include "models/extracolumnsproxymodel.h"

class GraphDataModel;

/*!
 * \brief The signal list as the Signals dialog table shows it: the GraphDataModel columns plus a
 * Data point column with the adapter's description of each signal's data points.
 *
 * The extra column only exists in this view; the descriptions depend on running adapters and are
 * read from a DataPointDescriptionCache.
 */
class SignalTableProxyModel : public ExtraColumnsProxyModel
{
    Q_OBJECT

public:
    SignalTableProxyModel(GraphDataModel* pGraphDataModel,
                          DataPointDescriptionCache* pDescriptionCache,
                          QObject* parent = nullptr);

    int dataPointColumn() const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

protected:
    QVariant extraColumnData(int row, int extraColumn, int role) const override;
    QVariant extraHeaderData(int extraColumn, int role) const override;

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
    QVariant expressionToolTip(int row) const;
    QString invalidReferenceError(int row) const;

    GraphDataModel* _pGraphDataModel;
    DataPointDescriptionCache* _pDescriptionCache;
};

#endif // SIGNALTABLEPROXYMODEL_H
