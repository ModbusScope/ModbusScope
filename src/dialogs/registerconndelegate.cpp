#include <QComboBox>

#include "registerconndelegate.h"

RegisterConnDelegate::RegisterConnDelegate(SettingsModel * pSettingsModel, QObject *parent)
    : QStyledItemDelegate(parent)
{
    _pSettingsModel = pSettingsModel;
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

    for (quint8 i = 0u; i < SettingsModel::CONNECTION_ID_CNT; i++)
    {
        if (_pSettingsModel->connectionState(i))
        {
            cb->addItem(QString(tr("Connection %1").arg(i + 1)), i);
        }
    }

    return cb;
}

void RegisterConnDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(cb);

    // get the index of the text in the combobox that matches the current value of the item
    const quint8 cbIndex = static_cast<quint8>(index.data(Qt::EditRole).toUInt());

    cb->setCurrentIndex(cbIndex);

    cb->showPopup();
}

void RegisterConnDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(cb);

    model->setData(index, cb->currentData().toUInt(), Qt::EditRole);
}

void RegisterConnDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
