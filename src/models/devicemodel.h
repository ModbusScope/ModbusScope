#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H

#include <QAbstractTableModel>

#include "settingsmodel.h"

class DeviceModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit DeviceModel(SettingsModel* pSettingsModel, QObject* parent = nullptr);

    // Define column indices and names for Device fields
    enum DeviceColumns
    {
        DeviceIdColumn = 0,
        NameColumn,
        ConnectionIdColumn,
        SlaveIdColumn,
        ConsecutiveMaxColumn,
        Int32LittleEndianColumn,
        ColumnCount
    };

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

private:
    SettingsModel* _pSettingsModel;
};

#endif // DEVICEMODEL_H
