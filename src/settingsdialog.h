#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <modbussettings.h>
#include <QStringListModel>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();
    void getModbusSettings(ModbusSettings * pSettings);

public slots:
    void accept();
    void reject();

private slots:
    void addRegister();
    void removeRegister();

private:
    Ui::SettingsDialog *ui;
    ModbusSettings commSettings;

    QStringListModel modelReg;

};

#endif // SETTINGSDIALOG_H
