#ifndef EXTRACOLUMNSPROXYMODEL_H
#define EXTRACOLUMNSPROXYMODEL_H

#include <QIdentityProxyModel>
#include <QItemSelection>
#include <QList>
#include <QPersistentModelIndex>

/*!
 * \brief A table model that shows every column of its source plus columns of its own after them.
 *
 * Subclasses provide the contents of the extra columns with extraColumnData(); the extra columns
 * have no source index. Rows, source columns and their changes are passed on as by
 * QIdentityProxyModel; the extra columns follow because they are always after the source columns.
 * Source layout changes are handled here, so persistent indexes in extra columns stay on their row.
 *
 * Only flat (table) source models are supported: rows of the extra columns have no children.
 */
class ExtraColumnsProxyModel : public QIdentityProxyModel
{
    Q_OBJECT

public:
    explicit ExtraColumnsProxyModel(int extraColumnCount, QObject* parent = nullptr);

    int firstExtraColumn() const;
    int extraColumnIndex(int proxyColumn) const;

    void setSourceModel(QAbstractItemModel* pSourceModel) override;

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    QModelIndex sibling(int row, int column, const QModelIndex& idx) const override;
    QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;
    QItemSelection mapSelectionToSource(const QItemSelection& selection) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;

protected:
    virtual QVariant extraColumnData(int row, int extraColumn, int role) const = 0;
    virtual QVariant extraHeaderData(int extraColumn, int role) const;
    virtual Qt::ItemFlags extraColumnFlags(int row, int extraColumn) const;

    void extraColumnsChanged(int firstRow, int lastRow);

private slots:
    void onSourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex>& sourceParents,
                                        QAbstractItemModel::LayoutChangeHint hint);
    void onSourceLayoutChanged(const QList<QPersistentModelIndex>& sourceParents,
                               QAbstractItemModel::LayoutChangeHint hint);

private:
    QList<QPersistentModelIndex> proxyParents(const QList<QPersistentModelIndex>& sourceParents) const;

    int _extraColumnCount;

    //! Persistent proxy indexes during a source layout change, with the source index of their row
    //! (column 0 for an extra column) and their proxy column.
    QModelIndexList _layoutChangeProxyIndexes;
    QList<QPersistentModelIndex> _layoutChangeSourceIndexes;
    QList<int> _layoutChangeColumns;
};

#endif // EXTRACOLUMNSPROXYMODEL_H
