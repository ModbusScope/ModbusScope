
#include "projectfilehandler.h"

#include "importexport/projectfiledata.h"
#include "importexport/projectfilejsonexporter.h"
#include "importexport/projectfilejsonparser.h"
#include "importexport/projectfilexmlparser.h"
#include "models/adapterdata.h"
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
            parseErr =
              xmlParser.parseFile(projectFileContents, &loadedSettings, QFileInfo(projectFilePath).absolutePath());
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
        const QList<ProjectFileData::AdapterFileSettings> adapters = buildCurrentAdapters();
        const QList<ProjectFileData::DeviceSettings> devices = buildCurrentDevices(adapters);
        ProjectFileJsonExporter projectFileExporter(_pGuiModel, _pSettingsModel, _pGraphDataModel);
        projectFileExporter.exportProjectFile(projectFile, adapters, devices);
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
    applyAdapterSettings(pProjectSettings->general.adapterList);
    applyDeviceSettings(pProjectSettings->general.deviceSettings);
    applyLogSettings(pProjectSettings->general.logSettings);
    applyViewSettings(pProjectSettings->view);
    applyGraphData(pProjectSettings->scope);
}

/*!
 * \brief Apply loaded adapter settings to SettingsModel so the dialog shows the correct values.
 * \param adapters Adapter list parsed from the project file.
 */
void ProjectFileHandler::applyAdapterSettings(const QList<ProjectFileData::AdapterFileSettings>& adapters)
{
    for (const ProjectFileData::AdapterFileSettings& adapter : adapters)
    {
        _pSettingsModel->setAdapterCurrentConfig(adapter.type, adapter.settings);
    }
}

/*!
 * \brief Build the adapters list from live SettingsModel state, with fallback to stored data.
 *
 * Adapters that have a stored config in SettingsModel (set via the dialog or on load) are
 * written from the model. Any adapter present in the original file but not in the model is
 * preserved unchanged to avoid data loss.
 */
QList<ProjectFileData::AdapterFileSettings> ProjectFileHandler::buildCurrentAdapters()
{
    QList<ProjectFileData::AdapterFileSettings> result;
    QStringList handledTypes;

    for (const QString& adapterId : _pSettingsModel->adapterIds())
    {
        const AdapterData* pAdapter = _pSettingsModel->adapterData(adapterId);
        if (pAdapter->hasStoredConfig())
        {
            ProjectFileData::AdapterFileSettings settings;
            settings.type = adapterId;
            settings.settings = pAdapter->currentConfig();
            result.append(settings);
            handledTypes.append(adapterId);
        }
    }

    for (const ProjectFileData::AdapterFileSettings& stored : _storedAdapters)
    {
        if (!handledTypes.contains(stored.type))
        {
            result.append(stored);
        }
    }

    return result;
}

/*!
 * \brief Build the generic devices list from live SettingsModel state.
 * \param adapters The adapters list (used to compute the numeric adapterId index).
 */
QList<ProjectFileData::DeviceSettings> ProjectFileHandler::buildCurrentDevices(
  const QList<ProjectFileData::AdapterFileSettings>& adapters)
{
    QList<ProjectFileData::DeviceSettings> result;

    for (const deviceId_t devId : _pSettingsModel->deviceList())
    {
        Device* pDev = _pSettingsModel->deviceSettings(devId);
        ProjectFileData::DeviceSettings ds;
        ds.bDeviceId = true;
        ds.deviceId = devId;
        ds.bName = true;
        ds.name = pDev->name();
        ds.adapterType = pDev->adapterId();
        ds.adapterId = 0;
        for (int i = 0; i < adapters.size(); ++i)
        {
            if (adapters[i].type == ds.adapterType)
            {
                ds.adapterId = static_cast<quint32>(i);
                break;
            }
        }
        result.append(ds);
    }

    return result;
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
