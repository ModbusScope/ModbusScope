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
    void GetModbusSettings(ModbusSettings * pSettings);
    void GetRegisterList(QList <quint16> * pRegisterList);

public slots:
    void Accept();
    void Reject();

private slots:
    void AddRegister();
    void RemoveRegister();

private:
    Ui::SettingsDialog * _ui;
    ModbusSettings _commSettings;
    QList <quint16> _registerList;

    QStringListModel _modelReg;

};

#endif // SETTINGSDIALOG_H
