#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

// Forward decleration
class SettingsModel;

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(SettingsModel* pSettingsModel, QWidget* parent = nullptr);
    ~SettingsDialog();

private:
    Ui::SettingsDialog* _pUi;

    SettingsModel* _pSettingsModel;
};

#endif // SETTINGSDIALOG_H
