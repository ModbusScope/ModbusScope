#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QList>
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
    void getRegisterList(QList <quint16> * pRegisterList);

public slots:
    void accept();
    void reject();

private slots:
    void addRegister();
    void removeRegister();

private:
    Ui::SettingsDialog *ui;
    ModbusSettings commSettings;
    QList <quint16> registerList;

    QStringListModel modelReg;

};

#endif // SETTINGSDIALOG_H
