#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QMessageBox>
#include "QDebug"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::SettingsDialog)
{
    _ui->setupUi(this);

    _modelReg.insertRow(_modelReg.rowCount());
    QModelIndex index = _modelReg.index(_modelReg.rowCount() - 1);
    _modelReg.setData(index, "40001");

    _ui->listReg->setModel(&_modelReg);
    _ui->listReg->show();

    // Setup handler for buttons
    connect(_ui->btnAdd, SIGNAL(released()), this, SLOT(addRegister()));
    connect(_ui->btnRemove, SIGNAL(released()), this, SLOT(removeRegister()));
}

SettingsDialog::~SettingsDialog()
{
    delete _ui;
}

void SettingsDialog::addRegister()
{
    _modelReg.insertRow(_modelReg.rowCount());
    QModelIndex index = _modelReg.index(_modelReg.rowCount() - 1);
    _modelReg.setData(index, _ui->spinReg->text());
}


void SettingsDialog::removeRegister()
{
    _modelReg.removeRows(_ui->listReg->currentIndex().row(), 1);
}

void SettingsDialog::getModbusSettings(ModbusSettings * pSettings)
{
    pSettings->copy(&_commSettings);
}

void SettingsDialog::getRegisterList(QList <quint16> * pRegisterList)
{
    pRegisterList->clear();

    for(qint32 i = 0; i < _registerList.size(); i++)
    {
        pRegisterList->append(_registerList[i]);
    }
}

void SettingsDialog::accept()
{
    /* Process data */

    // TODO: add some checks */
    _commSettings.setIpAddress(_ui->lineIP->text());
    _commSettings.setPort(_ui->spinPort->text().toInt());
    _commSettings.setSlaveId(_ui->spinSlaveId->text().toInt());

    _registerList.clear();
    for(qint32 i = 0; i < _modelReg.rowCount(); i++)
    {
        _registerList.append(_modelReg.data(_modelReg.index(i), Qt::DisplayRole).toInt());
    }

    //if processing is ok
    QDialog::accept();
        // Close dialog


    //else
        /*
         * Show message with error and don't close dialog
         * */
}

void SettingsDialog::reject()
{
    QDialog::reject();
}

