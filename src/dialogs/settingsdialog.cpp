#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "dialogs/connectiondialog.h"
#include "dialogs/logdialog.h"
#include <qlabel.h>

SettingsDialog::SettingsDialog(SettingsModel* pSettingsModel, QWidget* parent)
    : QDialog(parent), _pUi(new Ui::SettingsDialog), _pSettingsModel(pSettingsModel)
{
    _pUi->setupUi(this);

    _pUi->settingsList->addItem("Connection");
    _pUi->settingsList->addItem("Log");

    _pUi->settingsStack->insertWidget(0, new ConnectionDialog(pSettingsModel));
    _pUi->settingsStack->insertWidget(1, new LogDialog(pSettingsModel));

    connect(_pUi->settingsList, &QListWidget::currentRowChanged, _pUi->settingsStack, &QStackedWidget::setCurrentIndex);
}

SettingsDialog::~SettingsDialog()
{
    delete _pUi;
}
