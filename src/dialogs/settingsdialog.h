#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QList>

// Forward declaration
class SettingsModel;
class AdapterSettings;
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
    void acceptAllValues();

    Ui::SettingsDialog* _pUi;

    SettingsModel* _pSettingsModel;
    QList<AdapterSettings*> _dynamicSettings;
    AdapterDeviceSettings* _pDevSettings;
};

#endif // SETTINGSDIALOG_H
