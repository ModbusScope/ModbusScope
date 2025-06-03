#include "actionbuttondelegate.h"

#include <QApplication>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>

// Based on example (2018) from https://forum.qt.io/post/475871
// Current limitation is that width of the button is limited to the height of the tableview row

ActionButtonDelegate::ActionButtonDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

void ActionButtonDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_ASSERT(index.isValid());
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    const QWidget* widget = option.widget;
    const QStyle* style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    QStyleOptionButton editButtonOption;
    editButtonOption.text = QString(QChar(0x2026));
    editButtonOption.rect = this->buttonRect(option);
    editButtonOption.features = QStyleOptionButton::None;
    editButtonOption.direction = option.direction;
    editButtonOption.fontMetrics = option.fontMetrics;
    editButtonOption.palette = option.palette;
    editButtonOption.styleObject = option.styleObject;
    style->drawControl(QStyle::CE_PushButton, &editButtonOption, painter, widget);
}

QSize ActionButtonDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QSize baseSize = QStyledItemDelegate::sizeHint(option, index);
    return QSize(baseSize.width() + baseSize.height(), baseSize.height());
}

bool ActionButtonDelegate::editorEvent(QEvent* event,
                                       QAbstractItemModel* model,
                                       const QStyleOptionViewItem& option,
                                       const QModelIndex& index)
{
    Q_UNUSED(model)

    if (event->type() == QEvent::MouseButtonRelease)
    {
        auto mouseEvent = static_cast<QMouseEvent const*>(event);
        if (mouseEvent->button() == Qt::LeftButton && this->buttonRect(option).contains(mouseEvent->pos()))
        {
            emit clicked(index.row());
            return true;
        }
    }

    return false;
}

QRect ActionButtonDelegate::buttonRect(QStyleOptionViewItem const& option) const
{
    return QRect(option.rect.left() + option.rect.width() - option.rect.height(), option.rect.top(),
                 option.rect.height(), option.rect.height());
}
