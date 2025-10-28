#include "devicemodel.h"
#include "customwidgets/centeredbox.h"
#include "settingsmodel.h"

#include <QStringList>

static const QStringList DeviceHeaderNames = { "Device ID", "Connection ID", "Slave ID", "Consecutive Max",
                                               "Int32 Little Endian" };

DeviceModel::DeviceModel(SettingsModel* pSettingsModel, QObject* parent)
    : QAbstractTableModel(parent), _pSettingsModel(pSettingsModel)
{
}

QVariant DeviceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if (orientation == Qt::Horizontal)
    {
        if (section >= 0 && section < DeviceHeaderNames.size())
        {
            return DeviceHeaderNames[section];
        }
    }
    return QVariant();
}

int DeviceModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    if (!_pSettingsModel)
        return 0;

    return _pSettingsModel->deviceList().size();
}

int DeviceModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return ColumnCount;
}

QVariant DeviceModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !_pSettingsModel)
        return QVariant();

    const QList<deviceId_t> devices = _pSettingsModel->deviceList();
    if (index.row() < 0 || index.row() >= devices.size())
        return QVariant();

    deviceId_t devId = devices.at(index.row());
    Device* device = _pSettingsModel->deviceSettings(devId);
    if (!device)
        return QVariant();

    switch (index.column())
    {
    case DeviceIdColumn:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return devId;
        }
        break;
    case ConnectionIdColumn:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            auto conn = QString(tr("Connection %1").arg(device->connectionId() + 1));
            return conn;
        }
        break;
    case SlaveIdColumn:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return device->slaveId();
        }
        break;
    case ConsecutiveMaxColumn:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return device->consecutiveMax();
        }
        break;
    case Int32LittleEndianColumn:
        if (role == Qt::CheckStateRole)
        {
            if (device->int32LittleEndian())
            {
                return Qt::Checked;
            }
            else
            {
                return Qt::Unchecked;
            }
        }
        else if (role == CheckAlignmentRole)
        {
            return Qt::AlignCenter;
        }
        else
        {
            return QVariant();
        }
        break;
    default:
        break;
    }

    return QVariant();
}

bool DeviceModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || !_pSettingsModel)
        return false;

    QList<deviceId_t> devices = _pSettingsModel->deviceList();
    if (index.row() < 0 || index.row() >= devices.size())
        return false;

    deviceId_t devId = devices.at(index.row());
    Device* device = _pSettingsModel->deviceSettings(devId);
    if (!device)
        return false;

    bool changed = false;
    switch (index.column())
    {
    case DeviceIdColumn:
    {
        if (role == Qt::EditRole)
        {
            bool bSuccess = false;
            deviceId_t newId = value.toUInt(&bSuccess);
            if (bSuccess && (devId != newId) && (!_pSettingsModel->deviceList().contains(newId)))
            {
                if (_pSettingsModel->updateDeviceId(devId, newId))
                {
                    changed = true;
                }
            }
        }
        break;
    }
    case ConnectionIdColumn:
        if (role == Qt::EditRole)
        {
            bool bSuccess = false;
            auto newConnId = static_cast<ConnectionTypes::connectionId_t>(value.toUInt(&bSuccess));

            if ((bSuccess) && (newConnId < ConnectionTypes::ID_CNT))
            {
                if (device->connectionId() != newConnId)
                {
                    device->setConnectionId(newConnId);
                    changed = true;
                }
            }
        }
        break;
    case SlaveIdColumn:
    {
        if (role == Qt::EditRole)
        {
            bool bSuccess = false;
            quint8 slaveId = value.toUInt(&bSuccess);
            if (bSuccess && device->slaveId() != slaveId)
            {
                device->setSlaveId(value.toUInt());
                changed = true;
            }
        }
    }
    break;
    case ConsecutiveMaxColumn:
    {
        if (role == Qt::EditRole)
        {
            bool bSuccess = false;
            quint8 consecutiveMax = value.toUInt(&bSuccess);
            if (bSuccess && (device->consecutiveMax() != consecutiveMax))
            {
                device->setConsecutiveMax(consecutiveMax);
                changed = true;
            }
        }
    }
    break;
    case Int32LittleEndianColumn:
        if (role == Qt::CheckStateRole)
        {
            bool newValue = value == Qt::Checked;
            if (device->int32LittleEndian() != newValue)
            {
                device->setInt32LittleEndian(newValue);
                changed = true;
            }
        }
        break;
    default:
        break;
    }

    if (changed)
    {
        emit dataChanged(index, index, { role });
        return true;
    }
    return false;
}

Qt::ItemFlags DeviceModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags itemFlags = Qt::NoItemFlags;

    itemFlags |= Qt::ItemIsEnabled;
    itemFlags |= Qt::ItemIsSelectable;

    if (index.column() == Int32LittleEndianColumn)
    {
        itemFlags |= Qt::ItemIsUserCheckable;
    }
    else
    {
        itemFlags |= Qt::ItemIsEditable;
    }

    return itemFlags;
}

bool DeviceModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if (!_pSettingsModel || row < 0 || count <= 0)
        return false;

    QList<deviceId_t> devices = _pSettingsModel->deviceList();
    int insertRow = qBound(0, row, devices.size());

    beginInsertRows(parent, insertRow, insertRow + count - 1);
    for (int i = 0; i < count; ++i)
    {
        // Find a free deviceId (simple approach: max+1)
        deviceId_t newId = 1;
        if (!devices.isEmpty())
            newId = *std::max_element(devices.begin(), devices.end()) + 1;
        while (devices.contains(newId))
            ++newId;

        _pSettingsModel->addDevice(newId);
        devices.append(newId);
    }
    endInsertRows();
    return true;
}

bool DeviceModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (!_pSettingsModel || row < 0 || count <= 0)
        return false;

    QList<deviceId_t> devices = _pSettingsModel->deviceList();
    if (row + count > devices.size())
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i)
    {
        deviceId_t devId = devices.at(row); // Always at 'row' since list shrinks
        _pSettingsModel->removeDevice(devId);
        devices.removeAt(row);
    }
    endRemoveRows();
    return true;
}
