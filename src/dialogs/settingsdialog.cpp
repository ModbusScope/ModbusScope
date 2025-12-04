#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "dialogs/connectionsettings.h"
#include "dialogs/devicesettings.h"
#include "dialogs/logsettings.h"
#include <qlabel.h>

SettingsDialog::SettingsDialog(SettingsModel* pSettingsModel, QWidget* parent)
    : QDialog(parent), _pUi(new Ui::SettingsDialog), _pSettingsModel(pSettingsModel)
{
    _pUi->setupUi(this);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto font = _pUi->settingsList->font();
    font.setPointSize(font.pointSize() * 1.2);
    _pUi->settingsList->setFont(font);
    _pUi->settingsList->setStyleSheet("QListWidget {padding: 10px;} QListWidget::item { margin: 10px; }");

    auto listItem = new QListWidgetItem(_pUi->settingsList);
    listItem->setText("Connections");
    listItem->setIcon(QIcon(":/menu_icon/icons/network.svg"));
    _pUi->settingsList->addItem(listItem);

    listItem = new QListWidgetItem(_pUi->settingsList);
    listItem->setText("Devices");
    listItem->setIcon(QIcon(":/menu_icon/icons/circuit-board.svg"));
    _pUi->settingsList->addItem(listItem);

    listItem = new QListWidgetItem(_pUi->settingsList);
    listItem->setText("Log");
    listItem->setIcon(QIcon(":/menu_icon/icons/clipboard-list.svg"));
    _pUi->settingsList->addItem(listItem);

    _pConnSettings = new ConnectionSettings(_pSettingsModel);
    _pDevSettings = new DeviceSettings(_pSettingsModel);

    _pUi->settingsStack->insertWidget(PAGE_CONNECTION, _pConnSettings);
    _pUi->settingsStack->insertWidget(PAGE_DEVICE, _pDevSettings);
    _pUi->settingsStack->insertWidget(PAGE_LOG, new LogSettings(_pSettingsModel));

    connect(_pUi->settingsList, &QListWidget::currentRowChanged, this, &SettingsDialog::settingsStackSwitch);
}

SettingsDialog::~SettingsDialog()
{
    delete _pUi;
}

void SettingsDialog::done(int r)
{
    Q_UNUSED(r);

    _pConnSettings->acceptValues();

    QDialog::done(QDialog::Accepted);
}

void SettingsDialog::settingsStackSwitch(int newRow)
{
    auto previousTab = _pUi->settingsStack->currentIndex();

    // Update connection settings when switching away from tab
    if (previousTab == PAGE_CONNECTION)
    {
        _pConnSettings->acceptValues();
    }

    // Validate device settings when switching to tab
    if (newRow == PAGE_DEVICE)
    {
        _pDevSettings->settingsTabsSwitched();
    }

    _pUi->settingsStack->setCurrentIndex(newRow);
}
