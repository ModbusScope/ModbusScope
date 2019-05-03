#include "registerconndelegate.h"
#include <QComboBox>

RegisterConnDelegate::RegisterConnDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

RegisterConnDelegate::~RegisterConnDelegate()
{
}

QWidget *RegisterConnDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    // Create the combobox and populate it
    QComboBox *cb = new QComboBox(parent);
    cb->addItem(tr("Connection 1"));
    cb->addItem(tr("Connection 2"));
    return cb;
}

void RegisterConnDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(cb);

    // get the index of the text in the combobox that matches the current value of the item
    const quint8 cbIndex = static_cast<quint8>(index.data(Qt::EditRole).toUInt());

    cb->setCurrentIndex(cbIndex);
}

void RegisterConnDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(cb);
    model->setData(index, cb->currentIndex(), Qt::EditRole);
}

void RegisterConnDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
