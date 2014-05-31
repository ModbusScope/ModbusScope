#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QMessageBox>
#include "QDebug"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    modelReg.insertRow(modelReg.rowCount());
    QModelIndex index = modelReg.index(modelReg.rowCount() - 1);
    modelReg.setData(index, "40001");

    ui->listReg->setModel(&modelReg);
    ui->listReg->show();

    // Setup handler for buttons
    connect(ui->btnAdd, SIGNAL(released()), this, SLOT(addRegister()));
    connect(ui->btnRemove, SIGNAL(released()), this, SLOT(removeRegister()));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::addRegister()
{
    modelReg.insertRow(modelReg.rowCount());
    QModelIndex index = modelReg.index(modelReg.rowCount() - 1);
    modelReg.setData(index, ui->spinReg->text());
}


void SettingsDialog::removeRegister()
{
    modelReg.removeRows(ui->listReg->currentIndex().row(), 1);
}

void SettingsDialog::getModbusSettings(ModbusSettings * pSettings)
{
    pSettings->Copy(&commSettings);
}

void SettingsDialog::getRegisterList(QList <quint16> * pRegisterList)
{
    pRegisterList->clear();

    for(qint32 i = 0; i < registerList.size(); i++)
    {
        pRegisterList->append(registerList[i]);
    }
}

void SettingsDialog::accept()
{
    /* Process data */

    // TODO: add some checks */
    commSettings.SetIpAddress(ui->lineIP->text());
    commSettings.SetPort(ui->spinPort->text().toInt());

    qDebug() << "Number of regs (" << modelReg.rowCount() << ")" << "\n";

    registerList.clear();
    for(int32_t i = 0; i < modelReg.rowCount(); i++)
    {
        registerList.append(modelReg.data(modelReg.index(i), Qt::DisplayRole).toInt());
    }
    qDebug() << "Number of regs 2 (" << registerList.size() << ")" << "\n";

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

