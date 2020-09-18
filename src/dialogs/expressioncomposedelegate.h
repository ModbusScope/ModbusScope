#ifndef EXPRESSIONCOMPOSEDELEGATE_H
#define EXPRESSIONCOMPOSEDELEGATE_H

#include <QStyledItemDelegate>

class ExpressionComposeDelegate : public QStyledItemDelegate
{
public: 
    ExpressionComposeDelegate(QObject *parent = nullptr);
    ~ExpressionComposeDelegate() override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const override;

};

#endif // EXPRESSIONCOMPOSEDELEGATE_H
