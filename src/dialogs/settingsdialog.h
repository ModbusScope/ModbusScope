#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

// Forward decleration
class SettingsModel;
class ConnectionSettings;

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(SettingsModel* pSettingsModel, QWidget* parent = nullptr);
    ~SettingsDialog();

private slots:
    void done(int r);
    void settingsStackSwitch(int currentRow);

private:
    Ui::SettingsDialog* _pUi;

    SettingsModel* _pSettingsModel;
    ConnectionSettings* _pConnSettings;
};

#endif // SETTINGSDIALOG_H
