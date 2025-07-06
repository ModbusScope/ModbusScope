#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "dialogs/connectionsettings.h"
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
    listItem->setText("Connection");
    listItem->setIcon(QIcon(":/menu_icon/icons/network.svg"));
    _pUi->settingsList->addItem(listItem);

    listItem = new QListWidgetItem(_pUi->settingsList);
    listItem->setText("Log");
    listItem->setIcon(QIcon(":/menu_icon/icons/clipboard-list.svg"));
    _pUi->settingsList->addItem(listItem);

    _pConnSettings = new ConnectionSettings(_pSettingsModel);

    _pUi->settingsStack->insertWidget(0, _pConnSettings);
    _pUi->settingsStack->insertWidget(1, new LogSettings(pSettingsModel));

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

void SettingsDialog::settingsStackSwitch(int currentRow)
{
    _pConnSettings->acceptValues();

    _pUi->settingsStack->setCurrentIndex(currentRow);
}
