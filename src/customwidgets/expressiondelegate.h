#ifndef EXPRESSIONDELEGATE_H
#define EXPRESSIONDELEGATE_H

#include <QStyledItemDelegate>

class ExpressionDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ExpressionDelegate(QObject* parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

signals:
    void clicked(int row);

private:
    QRect buttonRect(const QStyleOptionViewItem &option) const;

};

#endif // EXPRESSIONDELEGATE_H
