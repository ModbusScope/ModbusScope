
#include "projectfilehandler.h"

#include "importexport/projectfiledata.h"
#include "importexport/projectfilejsonexporter.h"
#include "importexport/projectfilejsonparser.h"
#include "importexport/projectfilexmlparser.h"
#include "models/device.h"
#include "models/graphdatamodel.h"
#include "models/guimodel.h"
#include "models/settingsmodel.h"
#include "util/fileselectionhelper.h"
#include "util/util.h"

#include <QFile>
#include <QFileDialog>

ProjectFileHandler::ProjectFileHandler(GuiModel* pGuiModel,
                                       SettingsModel* pSettingsModel,
                                       GraphDataModel* pGraphDataModel)
    : QObject(nullptr)
{
    _pGuiModel = pGuiModel;
    _pSettingsModel = pSettingsModel;
    _pGraphDataModel = pGraphDataModel;
}

void ProjectFileHandler::openProjectFile(QString projectFilePath)
{
    ProjectFileData::ProjectSettings loadedSettings;
    QFile file(projectFilePath);

    /* If we can't open it, let's show an error message. */
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&file);
        QString projectFileContents = in.readAll();

        GeneralError parseErr;
        QString trimmed = projectFileContents.trimmed();
        if (trimmed.startsWith('{'))
        {
            ProjectFileJsonParser jsonParser;
            parseErr = jsonParser.parseFile(projectFileContents, &loadedSettings);
        }
        else if (trimmed.startsWith('<'))
        {
            ProjectFileXmlParser xmlParser;
            parseErr = xmlParser.parseFile(projectFileContents, &loadedSettings);
        }
        else
        {
            parseErr.reportError(tr("The file is not a valid MBS project file."));
        }

        if (parseErr.result())
        {
            _storedAdapters = loadedSettings.general.adapterList;
            _storedDevices = loadedSettings.general.deviceSettings;

            this->updateProjectSetting(&loadedSettings);

            _pGuiModel->setProjectFilePath(projectFilePath);
            _pGuiModel->setGuiState(GuiModel::GuiState::STOPPED);
        }
        else
        {
            Util::showError(parseErr.msg());
        }
    }
    else
    {
        Util::showError(tr("Couldn't open project file: %1").arg(projectFilePath));
    }
}

void ProjectFileHandler::selectProjectSaveFile()
{
    QFileDialog dialog;
    FileSelectionHelper::configureFileDialog(&dialog, FileSelectionHelper::DIALOG_TYPE_SAVE,
                                             FileSelectionHelper::FILE_TYPE_MBS);

    QString selectedFile = FileSelectionHelper::showDialog(&dialog);
    if (!selectedFile.isEmpty())
    {
        _pGuiModel->setProjectFilePath(selectedFile);

        saveProjectFile();
    }
}

void ProjectFileHandler::saveProjectFile()
{
    QString projectFile = _pGuiModel->projectFilePath();
    if (!projectFile.isEmpty())
    {
        ProjectFileJsonExporter projectFileExporter(_pGuiModel, _pSettingsModel, _pGraphDataModel);
        projectFileExporter.exportProjectFile(projectFile, _storedAdapters, _storedDevices);
    }
}

void ProjectFileHandler::selectProjectOpenFile()
{
    QFileDialog dialog;
    FileSelectionHelper::configureFileDialog(&dialog, FileSelectionHelper::DIALOG_TYPE_OPEN,
                                             FileSelectionHelper::FILE_TYPE_MBS);

    QString selectedFile = FileSelectionHelper::showDialog(&dialog);
    if (!selectedFile.isEmpty())
    {
        this->openProjectFile(selectedFile);
    }
}

void ProjectFileHandler::reloadProjectFile()
{
    this->openProjectFile(_pGuiModel->projectFilePath());
}

void ProjectFileHandler::updateProjectSetting(ProjectFileData::ProjectSettings* pProjectSettings)
{
    applyDeviceSettings(pProjectSettings->general.deviceSettings);
    applyLogSettings(pProjectSettings->general.logSettings);
    applyViewSettings(pProjectSettings->view);
    applyGraphData(pProjectSettings->scope);
}

void ProjectFileHandler::applyLogSettings(const ProjectFileData::LogSettings& logSettings)
{
    if (logSettings.bPollTime)
    {
        _pSettingsModel->setPollTime(logSettings.pollTime);
    }

    _pSettingsModel->setAbsoluteTimes(logSettings.bAbsoluteTimes);

    _pSettingsModel->setWriteDuringLog(logSettings.bLogToFile);
    if (logSettings.bLogToFileFile)
    {
        _pSettingsModel->setWriteDuringLogFile(logSettings.logFile);
    }
    else
    {
        _pSettingsModel->setWriteDuringLogFileToDefault();
    }
}

void ProjectFileHandler::applyViewSettings(const ProjectFileData::ViewSettings& viewSettings)
{
    if (viewSettings.scaleSettings.xAxis.bSliding)
    {
        _pGuiModel->setxAxisSlidingInterval(static_cast<qint32>(viewSettings.scaleSettings.xAxis.slidingInterval));
        _pGuiModel->setxAxisScale(AxisMode::SCALE_SLIDING);
    }
    else
    {
        _pGuiModel->setxAxisScale(AxisMode::SCALE_AUTO);
    }

    if (viewSettings.scaleSettings.yAxis.bMinMax)
    {
        _pGuiModel->setyAxisMin(viewSettings.scaleSettings.yAxis.scaleMin);
        _pGuiModel->setyAxisMax(viewSettings.scaleSettings.yAxis.scaleMax);
        _pGuiModel->setyAxisScale(AxisMode::SCALE_MINMAX);
    }
    else if (viewSettings.scaleSettings.yAxis.bWindowScale)
    {
        _pGuiModel->setyAxisScale(AxisMode::SCALE_WINDOW_AUTO);
    }
    else
    {
        _pGuiModel->setyAxisScale(AxisMode::SCALE_AUTO);
    }

    if (viewSettings.scaleSettings.y2Axis.bMinMax)
    {
        _pGuiModel->sety2AxisMin(viewSettings.scaleSettings.y2Axis.scaleMin);
        _pGuiModel->sety2AxisMax(viewSettings.scaleSettings.y2Axis.scaleMax);
        _pGuiModel->sety2AxisScale(AxisMode::SCALE_MINMAX);
    }
    else if (viewSettings.scaleSettings.y2Axis.bWindowScale)
    {
        _pGuiModel->sety2AxisScale(AxisMode::SCALE_WINDOW_AUTO);
    }
    else
    {
        _pGuiModel->sety2AxisScale(AxisMode::SCALE_AUTO);
    }
}

void ProjectFileHandler::applyDeviceSettings(const QList<ProjectFileData::DeviceSettings>& deviceSettings)
{
    _pSettingsModel->removeAllDevice();

    if (deviceSettings.isEmpty())
    {
        return;
    }

    for (const ProjectFileData::DeviceSettings& devSettings : deviceSettings)
    {
        if (!devSettings.bDeviceId)
        {
            continue;
        }

        const QString adapterId = devSettings.adapterType.isEmpty() ? QString("modbus") : devSettings.adapterType;

        _pSettingsModel->addDevice(devSettings.deviceId);
        Device* pDev = _pSettingsModel->deviceSettings(devSettings.deviceId);
        if (devSettings.bName)
        {
            pDev->setName(devSettings.name);
        }
        pDev->setAdapterId(adapterId);
    }
}

void ProjectFileHandler::applyGraphData(const ProjectFileData::ScopeSettings& scopeSettings)
{
    _pGraphDataModel->clear();
    for (qint32 i = 0; i < scopeSettings.registerList.size(); i++)
    {
        GraphData rowData;
        ProjectFileData::RegisterSettings const* const pSettingData = &scopeSettings.registerList[i];

        rowData.setActive(pSettingData->bActive);
        rowData.setLabel(pSettingData->text);
        rowData.setColor(pSettingData->color);
        rowData.setValueAxis(pSettingData->valueAxis == 1 ? GraphData::VALUE_AXIS_SECONDARY
                                                          : GraphData::VALUE_AXIS_PRIMARY);
        rowData.setExpression(pSettingData->expression);

        _pGraphDataModel->add(rowData);
    }
}
