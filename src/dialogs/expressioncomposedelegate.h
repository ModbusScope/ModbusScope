#ifndef EXPRESSIONCOMPOSEDELEGATE_H
#define EXPRESSIONCOMPOSEDELEGATE_H

#include <QStyledItemDelegate>

/* Forward declaration */
class GraphDataModel;

class ExpressionComposeDelegate : public QStyledItemDelegate
{
public: 
    ExpressionComposeDelegate(GraphDataModel *pGraphDataModel, QObject *parent = nullptr);
    ~ExpressionComposeDelegate() override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const override;

private slots:
    void handleComposeButtonClicked();

private:

    GraphDataModel * _pGraphDataModel;

    static const QString _cButtonRowProperty;

};

#endif // EXPRESSIONCOMPOSEDELEGATE_H
