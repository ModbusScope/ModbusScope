#ifndef ACTIONBUTTONDELEGATE_H
#define ACTIONBUTTONDELEGATE_H

#include <QStyledItemDelegate>

class ActionButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ActionButtonDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void setCharacter(QChar character);

    bool editorEvent(QEvent* event,
                     QAbstractItemModel* model,
                     const QStyleOptionViewItem& option,
                     const QModelIndex& index) override;

signals:
    void clicked(const QModelIndex& index);

private:
    QRect buttonRect(const QStyleOptionViewItem& option) const;

    QChar _character;
};

#endif // ACTIONBUTTONDELEGATE_H
