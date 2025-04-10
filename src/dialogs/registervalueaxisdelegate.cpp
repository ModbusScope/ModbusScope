#include "registervalueaxisdelegate.h"

#include <QComboBox>

#include "models/graphdata.h"

RegisterValueAxisDelegate::RegisterValueAxisDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

RegisterValueAxisDelegate::~RegisterValueAxisDelegate()
{
}

QWidget *RegisterValueAxisDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    // Create the combobox and populate it
    QComboBox *cb = new QComboBox(parent);

    for (quint8 i = 0u; i < GraphData::VALUE_AXIS_CNT; i++)
    {
        QString axis = i == GraphData::VALUE_AXIS_PRIMARY ? "Y1" : "Y2";
        cb->addItem(axis, i);
    }

    return cb;
}

void RegisterValueAxisDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(cb);

    // get the index of the text in the combobox that matches the current value of the item
    const quint8 cbIndex = static_cast<quint8>(index.data(Qt::EditRole).toUInt());

    cb->setCurrentIndex(cbIndex);

    cb->showPopup();
}

void RegisterValueAxisDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(cb);

    model->setData(index, cb->currentData().toUInt(), Qt::EditRole);
}

void RegisterValueAxisDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
