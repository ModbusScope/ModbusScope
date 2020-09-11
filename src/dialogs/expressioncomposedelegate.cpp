#include "expressioncomposedelegate.h"

#include <QPushButton>
#include <QApplication>

ExpressionComposeDelegate::ExpressionComposeDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

ExpressionComposeDelegate::~ExpressionComposeDelegate()
{
}

void ExpressionComposeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    if (!(option.state & QStyle::State_Selected))
    {
        return;
    }

    QStyleOptionButton editButtonOption;
    editButtonOption.text = QStringLiteral("...");
    editButtonOption.rect = QRect(option.rect.left() + option.rect.width() - option.rect.height(), option.rect.top(), option.rect.height(), option.rect.height());
    editButtonOption.features = QStyleOptionButton::None;
    editButtonOption.direction = option.direction;
    editButtonOption.fontMetrics = option.fontMetrics;
    editButtonOption.palette = option.palette;
    editButtonOption.styleObject = option.styleObject;

    style->drawControl(QStyle::CE_PushButton, &editButtonOption, painter, widget);
}

QSize ExpressionComposeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QSize baseSize = QStyledItemDelegate::sizeHint(option, index);
    return QSize(baseSize.width() + baseSize.height(), baseSize.height());
}

QWidget* ExpressionComposeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget* result = new QWidget(parent);
    result->setGeometry(option.rect);

    QWidget* baseEditor = QStyledItemDelegate::createEditor(result, option, index);
    baseEditor->setObjectName("baseEditor");
    baseEditor->setGeometry(0, 0, option.rect.width() - option.rect.height(), option.rect.height());

    QPushButton* editButton = new QPushButton(QStringLiteral("..."), result);
    editButton->setObjectName("editButton");
    editButton->setGeometry(option.rect.width() - option.rect.height(), 0, option.rect.height(), option.rect.height());
    connect(editButton, &QPushButton::clicked, []() { qDebug("Edit"); } );

    return result;
}

void ExpressionComposeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QWidget* baseEditor = editor->findChild<QWidget*>("baseEditor");
    Q_ASSERT(baseEditor);

    QStyledItemDelegate::setEditorData(baseEditor, index);
}

void ExpressionComposeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QWidget* baseEditor = editor->findChild<QWidget *>("baseEditor");
    Q_ASSERT(baseEditor);

    QStyledItemDelegate::setModelData(baseEditor, model, index);
}

void ExpressionComposeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);

    QWidget* baseEditor = editor->findChild<QWidget *>("baseEditor");
    Q_ASSERT(baseEditor);
    baseEditor->setGeometry(0, 0, option.rect.width() - option.rect.height(), option.rect.height());

    QWidget* editButton = editor->findChild<QWidget *>("editButton");
    Q_ASSERT(editButton);
    editButton->setGeometry(option.rect.width() - option.rect.height(), 0, option.rect.height(), option.rect.height());
    editor->setGeometry(option.rect);
}

