#ifndef VALUEAXISDELEGATE_H
#define VALUEAXISDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

class ValueAxisDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ValueAxisDelegate(QObject* parent = nullptr);
    ~ValueAxisDelegate() override;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& /* index */) const override;

private:
};

#endif // VALUEAXISDELEGATE_H
