#include <QApplication>
#include <QPushButton>

#include "expressiondelegate.h"

// Based on example (2018) from https://forum.qt.io/post/475871
// Current limitation is that width of the button is limited to the height of the tableview row

ExpressionDelegate::ExpressionDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{

}
#if 0
void ExpressionDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    if (!(option.state & QStyle::State_Selected)){
        return;
    }

    QStyleOptionButton removeButtonOption;
    removeButtonOption.text = QString(
        QChar(0x270D)); //use emoji for text, optionally you can use icon + iconSize
    removeButtonOption.rect = QRect(option.rect.left() + option.rect.width() - option.rect.height(),
                                    option.rect.top(),
                                    option.rect.height(),
                                    option.rect.height());
    removeButtonOption.features = QStyleOptionButton::None;
    removeButtonOption.direction = option.direction;
    removeButtonOption.fontMetrics = option.fontMetrics;
    removeButtonOption.palette = option.palette;
    removeButtonOption.styleObject = option.styleObject;

    style->drawControl(QStyle::CE_PushButton, &removeButtonOption, painter, widget);
}
#endif

QSize ExpressionDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    const QSize baseSize = QStyledItemDelegate::sizeHint(option, index);
    return QSize(baseSize.width() + baseSize.height(), baseSize.height());
}

QWidget *ExpressionDelegate::createEditor(QWidget *parent,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    QWidget *result = new QWidget(parent);
    result->setGeometry(option.rect);
    QWidget *baseEditor = QStyledItemDelegate::createEditor(result, option, index);
    baseEditor->setObjectName("baseEditor");
    baseEditor->setGeometry(0, 0, option.rect.width() - option.rect.height(), option.rect.height());
    QPushButton *removeButton = new QPushButton(QChar(0x274C), result);
    removeButton->setObjectName("removeButton");
    removeButton->setGeometry(option.rect.width() - option.rect.height(),
                              0,
                              option.rect.height(),
                              option.rect.height());
    connect(removeButton, &QPushButton::clicked, []() { qDebug("Remove"); });
    return result;
}

void ExpressionDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QWidget *baseEditor = editor->findChild<QWidget *>("baseEditor");
    Q_ASSERT(baseEditor);
    QStyledItemDelegate::setEditorData(baseEditor, index);
}

void ExpressionDelegate::setModelData(QWidget *editor,
                                      QAbstractItemModel *model,
                                      const QModelIndex &index) const
{
    QWidget *baseEditor = editor->findChild<QWidget *>("baseEditor");
    Q_ASSERT(baseEditor);
    QStyledItemDelegate::setModelData(baseEditor, model, index);
}

void ExpressionDelegate::updateEditorGeometry(QWidget *editor,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
    QWidget *baseEditor = editor->findChild<QWidget *>("baseEditor");
    Q_ASSERT(baseEditor);
    baseEditor->setGeometry(0, 0, option.rect.width() - option.rect.height(), option.rect.height());
    QWidget *removeButton = editor->findChild<QWidget *>("removeButton");
    Q_ASSERT(removeButton);
    removeButton->setGeometry(option.rect.width() - option.rect.height(),
                              0,
                              option.rect.height(),
                              option.rect.height());
    editor->setGeometry(option.rect);
}
