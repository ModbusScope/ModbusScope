#include <QApplication>
#include <QPushButton>
#include <QLineEdit>

#include "expressiondelegate.h"

// Based on example (2018) from https://forum.qt.io/post/475871
// Current limitation is that width of the button is limited to the height of the tableview row

ExpressionDelegate::ExpressionDelegate(GraphDataModel *pGraphDataModel, QObject *parent)
    : QStyledItemDelegate{parent}
{
    _pGraphDataModel = pGraphDataModel;
}

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
    QPushButton *editButton = new QPushButton(QChar(0x274C), result);
    editButton->setObjectName("editButton");
    editButton->setGeometry(option.rect.width() - option.rect.height(),
                              0,
                              option.rect.height(),
                              option.rect.height());
    editButton->setProperty("RowIndex", index.row());
    connect(editButton, &QPushButton::clicked, this, &ExpressionDelegate::handleClicked);

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

    QLineEdit *editLine = static_cast<QLineEdit*>(baseEditor);

    model->setData(index, editLine->text(), Qt::EditRole);
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
    QWidget *editButton = editor->findChild<QWidget *>("editButton");
    Q_ASSERT(editButton);
    editButton->setGeometry(option.rect.width() - option.rect.height(),
                              0,
                              option.rect.height(),
                              option.rect.height());
    editor->setGeometry(option.rect);
}

void ExpressionDelegate::handleClicked()
{
    QObject* pObj = sender();
    auto btn = static_cast<QWidget*>(pObj);

    //NEED to accept current value before opening dialog

    int row = btn->property("RowIndex").value<int>();

    emit clicked(row);
}
