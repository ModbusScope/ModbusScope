#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

// Forward declaration
class SettingsModel;
class AdapterConnectionSettings;
class AdapterDeviceSettings;

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
    void settingsStackSwitch(int newRow);

private:
    enum
    {
        PAGE_CONNECTION = 0,
        PAGE_DEVICE = 1,
        PAGE_LOG = 2,
    };

    Ui::SettingsDialog* _pUi;

    SettingsModel* _pSettingsModel;
    AdapterConnectionSettings* _pConnSettings;
    AdapterDeviceSettings* _pDevSettings;
};

#endif // SETTINGSDIALOG_H
