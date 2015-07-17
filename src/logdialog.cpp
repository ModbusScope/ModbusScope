#include "logdialog.h"
#include "ui_logdialog.h"


LogDialog::LogDialog(LogModel * pLogModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::LogDialog)
{
    _pUi->setupUi(this);

    _pLogModel = pLogModel;

    connect(_pLogModel, SIGNAL(pollTimeChanged()), this, SLOT(updatePollTime()));
}

LogDialog::~LogDialog()
{
    delete _pUi;
}

void LogDialog::updatePollTime()
{
    _pUi->spinPollTime->setValue(_pLogModel->pollTime());
}

void LogDialog::done(int r)
{
    bool bValid = true;

    if(QDialog::Accepted == r)  // ok was pressed
    {
        _pLogModel->setPollTime(_pUi->spinPollTime->text().toUInt());

        // Validate the data
        //bValid = validateSettingsData();
        bValid = true;
    }
    else
    {
        // cancel, close or exc was pressed
        bValid = true;
    }

    if (bValid)
    {
        QDialog::done(r);
    }
}
