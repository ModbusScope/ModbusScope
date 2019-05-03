#ifndef REGISTER_CONN_DELEGATE_H
#define REGISTER_CONN_DELEGATE_H

#include <QStyledItemDelegate>
#include "settingsmodel.h"

class RegisterConnDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    RegisterConnDelegate(SettingsModel * pSettingsModel, QObject *parent = nullptr);
    ~RegisterConnDelegate() override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const override;

private:
    SettingsModel * _pSettingsModel;
};

#endif // REGISTER_CONN_DELEGATE_H
