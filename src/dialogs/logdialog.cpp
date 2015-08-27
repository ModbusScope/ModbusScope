#include "logdialog.h"
#include "ui_logdialog.h"

#include "settingsmodel.h"
#include "guimodel.h"

#include <QFileDialog>

LogDialog::LogDialog(SettingsModel * pSettingsModel, GuiModel * pGuiModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::LogDialog)
{
    _pUi->setupUi(this);

    _pSettingsModel = pSettingsModel;
    _pGuiModel = pGuiModel;

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    /*-- View connections --*/
    connect(_pUi->checkWriteDuringLog, SIGNAL(toggled(bool)), _pSettingsModel, SLOT(setWriteDuringLog(bool)));
    connect(_pUi->buttonWriteDuringLogFile, SIGNAL(clicked()), this, SLOT(selectLogFile()));
    connect(_pUi->checkAbsoluteTimes, SIGNAL(toggled(bool)), _pSettingsModel, SLOT(setAbsoluteTimes(bool)));

    /*-- connect model to view --*/
    connect(_pSettingsModel, SIGNAL(pollTimeChanged()), this, SLOT(updatePollTime()));
    connect(_pSettingsModel, SIGNAL(writeDuringLogChanged()), this, SLOT(updateWriteDuringLog()));
    connect(_pSettingsModel, SIGNAL(writeDuringLogFileChanged()), this, SLOT(updateWriteDuringLogFile()));
    connect(_pSettingsModel, SIGNAL(absoluteTimesChanged()), this, SLOT(updateAbsoluteTime()));
}

LogDialog::~LogDialog()
{
    delete _pUi;
}

void LogDialog::done(int r)
{
    bool bValid = true;

    if(QDialog::Accepted == r)  // ok was pressed
    {
        _pSettingsModel->setPollTime(_pUi->spinPollTime->text().toUInt());
        _pSettingsModel->setWriteDuringLogFile(_pUi->lineWriteDuringLogFile->text());

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

void LogDialog::selectLogFile()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::HideNameFilterDetails, false);
    dialog.setDefaultSuffix("csv");
    dialog.setWindowTitle(tr("Select csv file"));
    dialog.setNameFilter(tr("CSV files (*.csv)"));
    dialog.setDirectory(_pGuiModel->lastDir());

    if (dialog.exec() == QDialog::Accepted)
    {
        _pUi->lineWriteDuringLogFile->setText(dialog.selectedFiles().first());
    }
}

void LogDialog::updatePollTime()
{
    _pUi->spinPollTime->setValue(_pSettingsModel->pollTime());
}

void LogDialog::updateWriteDuringLog()
{
    if (_pSettingsModel->writeDuringLog())
    {
        _pUi->checkWriteDuringLog->setChecked(true);
        _pUi->lineWriteDuringLogFile->setEnabled(true);
        _pUi->buttonWriteDuringLogFile->setEnabled(true);
    }
    else
    {
        _pUi->checkWriteDuringLog->setChecked(false);
        _pUi->lineWriteDuringLogFile->setEnabled(false);
        _pUi->buttonWriteDuringLogFile->setEnabled(false);
    }
}

void LogDialog::updateWriteDuringLogFile()
{
    _pUi->lineWriteDuringLogFile->setText(_pSettingsModel->writeDuringLogFile());
}

void LogDialog::updateAbsoluteTime()
{
    _pUi->checkAbsoluteTimes->setChecked(_pSettingsModel->absoluteTimes());
}


