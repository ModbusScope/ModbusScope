#ifndef EXPRESSIONDELEGATE_H
#define EXPRESSIONDELEGATE_H

#include <QStyledItemDelegate>

/* Forward declaration */
class GraphDataModel;

class ExpressionDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ExpressionDelegate(GraphDataModel* pGraphDataModel, QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

signals:
    void clicked(int row);

private slots:
    void handleClicked();

private:
    GraphDataModel* _pGraphDataModel;

};

#endif // EXPRESSIONDELEGATE_H
