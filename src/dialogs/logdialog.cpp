#include "logdialog.h"
#include "ui_logdialog.h"

#include "settingsmodel.h"
#include "guimodel.h"

#include <QFileDialog>
#include "fileselectionhelper.h"

LogDialog::LogDialog(SettingsModel * pSettingsModel, GuiModel * pGuiModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::LogDialog)
{
    _pUi->setupUi(this);

    _pSettingsModel = pSettingsModel;
    _pGuiModel = pGuiModel;

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    _pTimeReferenceGroup = new QButtonGroup();
    _pTimeReferenceGroup->setExclusive(true);
    _pTimeReferenceGroup->addButton(_pUi->radioAbsolute, 0);
    _pTimeReferenceGroup->addButton(_pUi->radioRelative, 1);
    connect(_pTimeReferenceGroup, &QButtonGroup::idClicked, this, &LogDialog::updateReferenceTime);

    /*-- View connections --*/
    connect(_pUi->checkWriteDuringLog, &QCheckBox::toggled, _pSettingsModel, &SettingsModel::setWriteDuringLog);
    connect(_pUi->buttonWriteDuringLogFile, &QToolButton::clicked, this, &LogDialog::selectLogFile);

    /*-- connect model to view --*/
    connect(_pSettingsModel, &SettingsModel::pollTimeChanged, this, &LogDialog::updatePollTime);
    connect(_pSettingsModel, &SettingsModel::writeDuringLogChanged, this, &LogDialog::updateWriteDuringLog);
    connect(_pSettingsModel, &SettingsModel::writeDuringLogFileChanged, this, &LogDialog::updateWriteDuringLogFile);
    connect(_pSettingsModel, &SettingsModel::absoluteTimesChanged, this, &LogDialog::timeReferenceUpdated);
}

LogDialog::~LogDialog()
{
    delete _pTimeReferenceGroup;
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
    FileSelectionHelper::configureFileDialog(&dialog, _pGuiModel,
                                             FileSelectionHelper::DIALOG_TYPE_SAVE,
                                             FileSelectionHelper::FILE_TYPE_CSV);

    if (dialog.exec() == QDialog::Accepted)
    {
        auto fileList = dialog.selectedFiles();
        if (!fileList.isEmpty())
        {
            _pUi->lineWriteDuringLogFile->setText(fileList.at(0));
        }
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

void LogDialog::timeReferenceUpdated()
{
    if (_pSettingsModel->absoluteTimes())
    {
        _pUi->radioAbsolute->setChecked(true);
    }
    else
    {
        _pUi->radioRelative->setChecked(true);
    }
}

void LogDialog::updateReferenceTime(int id)
{
    _pSettingsModel->setAbsoluteTimes(id == 0 ? true: false);
}


