#ifndef CONNECTION_DELEGATE_H
#define CONNECTION_DELEGATE_H

#include "models/settingsmodel.h"
#include <QStyledItemDelegate>

class ConnectionDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ConnectionDelegate(SettingsModel* pSettingsModel, QObject* parent = nullptr);
    ~ConnectionDelegate() override = default;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& /* index */) const override;

private:
    SettingsModel* _pSettingsModel;
};

#endif // CONNECTION_DELEGATE_H
