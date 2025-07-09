#include "logsettings.h"
#include "ui_logsettings.h"

#include "models/settingsmodel.h"
#include "util/fileselectionhelper.h"

#include <QFileDialog>

LogSettings::LogSettings(SettingsModel* pSettingsModel, QWidget* parent) : QWidget(parent), _pUi(new Ui::LogSettings)
{
    _pUi->setupUi(this);

    _pSettingsModel = pSettingsModel;

    _pTimeReferenceGroup = new QButtonGroup();
    _pTimeReferenceGroup->setExclusive(true);
    _pTimeReferenceGroup->addButton(_pUi->radioAbsolute, 0);
    _pTimeReferenceGroup->addButton(_pUi->radioRelative, 1);

    if (_pSettingsModel->absoluteTimes())
    {
        _pUi->radioAbsolute->setChecked(true);
    }
    else
    {
        _pUi->radioRelative->setChecked(true);
    }
    updateWriteDuringLog();
    _pUi->lineWriteDuringLogFile->setText(_pSettingsModel->writeDuringLogFile());
    _pUi->spinPollTime->setValue(_pSettingsModel->pollTime());

    /*-- View connections --*/
    connect(_pTimeReferenceGroup, &QButtonGroup::idClicked, this, &LogSettings::updateReferenceTime);
    connect(_pUi->checkWriteDuringLog, &QCheckBox::toggled, _pSettingsModel, &SettingsModel::setWriteDuringLog);
    connect(_pUi->buttonWriteDuringLogFile, &QToolButton::clicked, this, &LogSettings::selectLogFile);
    connect(_pUi->spinPollTime, QOverload<int>::of(&QSpinBox::valueChanged), this, &LogSettings::handlePollTime);
    connect(_pUi->lineWriteDuringLogFile, &QLineEdit::editingFinished, this, &LogSettings::handleLogFile);

    /*-- connect model to view --*/
    connect(_pSettingsModel, &SettingsModel::writeDuringLogChanged, this, &LogSettings::updateWriteDuringLog);
}

LogSettings::~LogSettings()
{
    delete _pTimeReferenceGroup;
    delete _pUi;
}

void LogSettings::selectLogFile()
{
    QFileDialog dialog(this);
    FileSelectionHelper::configureFileDialog(&dialog,
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

void LogSettings::handlePollTime(int pollTime)
{
    _pSettingsModel->setPollTime(pollTime);
}

void LogSettings::handleLogFile()
{
    _pSettingsModel->setWriteDuringLogFile(_pUi->lineWriteDuringLogFile->text());
}

void LogSettings::updateWriteDuringLog()
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

void LogSettings::updateReferenceTime(int id)
{
    _pSettingsModel->setAbsoluteTimes(id == 0 ? true: false);
}


