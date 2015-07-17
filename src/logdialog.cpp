#include "logdialog.h"
#include "ui_logdialog.h"

#include <QFileDialog>

LogDialog::LogDialog(LogModel * pLogModel, GuiModel * pGuiModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::LogDialog)
{
    _pUi->setupUi(this);

    _pLogModel = pLogModel;
    _pGuiModel = pGuiModel;

    /*-- View connections --*/
    connect(_pUi->checkWriteDuringLog, SIGNAL(toggled(bool)), _pLogModel, SLOT(setWriteDuringLog(bool)));
    connect(_pUi->buttonWriteDuringLogPath, SIGNAL(clicked()), this, SLOT(selectLogFile()));

    /*-- connect model to view --*/
    connect(_pLogModel, SIGNAL(pollTimeChanged()), this, SLOT(updatePollTime()));
    connect(_pLogModel, SIGNAL(writeDuringLogChanged()), this, SLOT(updateWriteDuringLog()));
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
        _pLogModel->setPollTime(_pUi->spinPollTime->text().toUInt());
        _pLogModel->setWriteDuringLogPath(_pUi->lineWriteDuringLogPath->text());

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
        _pUi->lineWriteDuringLogPath->setText(dialog.selectedFiles().first());
    }
}

void LogDialog::updatePollTime()
{
    _pUi->spinPollTime->setValue(_pLogModel->pollTime());
}

void LogDialog::updateWriteDuringLog()
{
    if (_pLogModel->writeDuringLog())
    {
        _pUi->lineWriteDuringLogPath->setEnabled(true);
        _pUi->buttonWriteDuringLogPath->setEnabled(true);
    }
    else
    {
        _pUi->lineWriteDuringLogPath->setEnabled(false);
        _pUi->buttonWriteDuringLogPath->setEnabled(false);
    }
}
