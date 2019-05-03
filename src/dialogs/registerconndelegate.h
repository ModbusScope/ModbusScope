#ifndef REGISTER_CONN_DELEGATE_H
#define REGISTER_CONN_DELEGATE_H

#include <QStyledItemDelegate>

class RegisterConnDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    RegisterConnDelegate(QObject *parent = nullptr);
    ~RegisterConnDelegate() override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const override;
};

#endif // REGISTER_CONN_DELEGATE_H
