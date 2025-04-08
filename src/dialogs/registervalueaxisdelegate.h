#ifndef REGISTER_VALUE_AXIS_DELEGATE_H
#define REGISTER_VALUE_AXIS_DELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

class RegisterValueAxisDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit RegisterValueAxisDelegate(QObject *parent = nullptr);
    ~RegisterValueAxisDelegate() override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const override;

private:

};

#endif // REGISTER_VALUE_AXIS_DELEGATE_H
