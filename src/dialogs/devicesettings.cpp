#include "devicesettings.h"
#include "dialogs/connectiondelegate.h"
#include "models/devicemodel.h"
#include "ui_devicesettings.h"

#include <QShortcut>

using DeviceColumns = DeviceModel::DeviceColumns;

DeviceSettings::DeviceSettings(SettingsModel* pSettingsModel, QWidget* parent)
    : QWidget(parent), _pUi(new Ui::DeviceSettings), _pSettingsModel(pSettingsModel)
{
    _pUi->setupUi(this);

    // Create and set up the device model
    _pDeviceModel = new DeviceModel(_pSettingsModel, this);
    _pUi->deviceView->setModel(_pDeviceModel);
    _pUi->deviceView->verticalHeader()->hide();
    _pUi->deviceView->setStyle(&_centeredBoxStyle);
    _pUi->deviceView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    _pUi->deviceView->setSelectionBehavior(QAbstractItemView::SelectRows);
    _pUi->deviceView->setSelectionMode(QAbstractItemView::SingleSelection);
    _pUi->deviceView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

    ConnectionDelegate* cbConn = new ConnectionDelegate(pSettingsModel, _pUi->deviceView);
    _pUi->deviceView->setItemDelegateForColumn(DeviceColumns::ConnectionIdColumn, cbConn);

    // Handle delete
    QShortcut* shortcut = new QShortcut(QKeySequence(QKeySequence::Delete), _pUi->deviceView);
    connect(shortcut, &QShortcut::activated, this, &DeviceSettings::onAddDeviceClicked);

    connect(_pUi->btnAdd, &QPushButton::clicked, this, &DeviceSettings::onAddDeviceClicked);
    connect(_pUi->btnRemove, &QPushButton::clicked, this, &DeviceSettings::onRemoveDeviceClicked);
}

DeviceSettings::~DeviceSettings()
{
    delete _pUi;
}

void DeviceSettings::onAddDeviceClicked()
{
    int row = _pDeviceModel->rowCount();
    _pDeviceModel->insertRow(row);
}

void DeviceSettings::onRemoveDeviceClicked()
{
    QModelIndexList selection = _pUi->deviceView->selectionModel()->selectedRows();
    // Remove from last to first to keep indices valid
    std::sort(selection.begin(), selection.end(),
              [](const QModelIndex& a, const QModelIndex& b) { return a.row() > b.row(); });
    for (const QModelIndex& idx : std::as_const(selection))
    {
        _pDeviceModel->removeRow(idx.row());
    }
}
