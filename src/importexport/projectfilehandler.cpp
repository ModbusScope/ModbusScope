
#include "projectfilehandler.h"

#include "ProtocolAdapter/adapterhub.h"
#include "importexport/legacy/projectfilexmlparser.h"
#include "importexport/projectfiledata.h"
#include "importexport/projectfilejsonexporter.h"
#include "importexport/projectfilejsonparser.h"
#include "models/adapterdata.h"
#include "models/device.h"
#include "models/graphdatamodel.h"
#include "models/guimodel.h"
#include "models/settingsmodel.h"
#include "util/fileselectionhelper.h"
#include "util/scopelogging.h"
#include "util/util.h"

#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>

#include <limits>
#include <utility>

ProjectFileHandler::ProjectFileHandler(GuiModel* pGuiModel,
                                       SettingsModel* pSettingsModel,
                                       GraphDataModel* pGraphDataModel,
                                       QObject* parent)
    : QObject(parent)
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

/*!
 * \brief Apply a parsed project file to the models.
 *
 * The adapter configs and the devices section are aligned before either reaches SettingsModel,
 * so the device list and the configs that are polled agree from the start, and the device list is
 * still applied as a single change.
 * \param pProjectSettings  The parsed project file.
 */
void ProjectFileHandler::updateProjectSetting(ProjectFileData::ProjectSettings* pProjectSettings)
{
    QList<ProjectFileData::AdapterFileSettings> adapters = pProjectSettings->general.adapterList;
    QMap<deviceId_t, Device> devices = buildDeviceList(pProjectSettings->general.deviceSettings);
    alignDevicesWithAdapterConfigs(adapters, devices);

    applyAdapterSettings(adapters);
    _pSettingsModel->applyDeviceList(devices);
    applyLogSettings(pProjectSettings->general.logSettings);
    applyViewSettings(pProjectSettings->view);
    applyGraphData(pProjectSettings->scope);
}

/*!
 * \brief Apply loaded adapter settings to SettingsModel so the dialog shows the correct values.
 *
 * Stores every device from the project file, even beyond an adapter's maxDevices()
 * limit: AdapterData::configForWire() caps what is actually sent to the adapter subprocess, so
 * truncating here would only silently drop devices from the UI without any indication to the
 * user.
 *
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

    const QStringList adapterIds = _pSettingsModel->adapterIds();
    for (const QString& adapterId : adapterIds)
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

    for (const ProjectFileData::AdapterFileSettings& stored : std::as_const(_storedAdapters))
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

    const QList<deviceId_t> devices = _pSettingsModel->deviceList();
    for (const deviceId_t devId : devices)
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

/*! \brief Build the device list from the devices section of the project file.
 *
 * A project file may name the same device ID more than once (e.g. a legacy XML file with two
 * <connection> tags that both default to id 0). Such a duplicate is merged rather than rejected,
 * field by field in file order: a later entry overrides only the fields it actually specifies,
 * so a missing name or adapter type keeps the value an earlier entry gave the device.
 * \param deviceSettings  The devices section of the project file. Entries without a device ID are skipped.
 * \return The devices keyed by device ID; empty when the section is empty.
 */
QMap<deviceId_t, Device> ProjectFileHandler::buildDeviceList(
  const QList<ProjectFileData::DeviceSettings>& deviceSettings)
{
    QMap<deviceId_t, Device> devices;

    for (const ProjectFileData::DeviceSettings& devSettings : deviceSettings)
    {
        if (!devSettings.bDeviceId)
        {
            continue;
        }

        const deviceId_t devId = devSettings.deviceId;
        if (devices.contains(devId))
        {
            qCWarning(scopeGeneralInfo) << qUtf8Printable(
              QString("ProjectFileHandler: duplicate device id %1 in project file - later entries "
                      "override the fields they specify")
                .arg(devId));
        }
        else
        {
            /* Set the fallback explicitly rather than leaning on Device's own default, so the
               adapter a file-loaded device falls back to is decided here and stays put if that
               default ever changes. */
            Device newDevice(devId);
            newDevice.setAdapterId(QString(cModbusAdapterId));
            devices.insert(devId, newDevice);
        }

        Device& device = devices[devId];
        if (devSettings.bName)
        {
            device.setName(devSettings.name);
        }
        if (!devSettings.adapterType.isEmpty())
        {
            device.setAdapterId(devSettings.adapterType);
        }
    }

    return devices;
}

/*! \brief Make the devices section and the adapter configs of a project file name the same devices.
 *
 * The adapter config is what gets polled, so it decides which devices exist and who owns them:
 * - A device a config lists but the devices section omits is added to \a devices.
 * - A device whose owner's config omits it, while another config lists it, moves to that adapter.
 * - A device no config lists is added to its owner's config as an entry carrying only its ID;
 *   AdapterData::effectiveConfig() completes that entry from the adapter's default device.
 *
 * Only adapters whose settings hold a "devices" array take part. Any other adapter, or one absent
 * from the file, takes its devices from the adapter defaults, which may not be known yet, so a device
 * it owns is left as it is; once that adapter describes, reconcileDevicesWithAdapters() settles it.
 * When several configs list a device, the first in alphabetical order wins, like the order
 * SettingsModel::reconcileDevicesWithAdapters() gives adapters with a stored config. Each
 * correction is logged; the file is not refused.
 * \param adapters  The adapters section; a missing device entry is appended to its owner's settings.
 * \param devices   The device list from buildDeviceList(); updated in place.
 */
void ProjectFileHandler::alignDevicesWithAdapterConfigs(QList<ProjectFileData::AdapterFileSettings>& adapters,
                                                        QMap<deviceId_t, Device>& devices)
{
    /* Adapter type -> index into adapters, for adapters that list their devices. A type that occurs
       more than once resolves to its last entry, the one applyAdapterSettings() leaves in the model.
       A QMap keeps the types in alphabetical order, which decides who wins a device listed by
       several configs. */
    QMap<QString, qsizetype> listingAdapters;
    for (qsizetype i = 0; i < adapters.size(); ++i)
    {
        if (adapters[i].settings.value("devices").isArray())
        {
            listingAdapters.insert(adapters[i].type, i);
        }
        else
        {
            listingAdapters.remove(adapters[i].type);
        }
    }

    const QMap<deviceId_t, QStringList> claims = collectDeviceClaims(adapters, listingAdapters);

    for (auto it = claims.cbegin(), end = claims.cend(); it != end; ++it)
    {
        const deviceId_t devId = it.key();
        const QString& firstClaimant = it.value().first();

        if (!devices.contains(devId))
        {
            Device newDevice(devId);
            newDevice.setAdapterId(firstClaimant);
            devices.insert(devId, newDevice);
            qCWarning(scopeGeneralInfo) << qUtf8Printable(
              QString("ProjectFileHandler: device %1 is listed by adapter '%2' but missing from the devices "
                      "section - added")
                .arg(devId)
                .arg(firstClaimant));
        }
        else
        {
            Device& device = devices[devId];
            if (listingAdapters.contains(device.adapterId()) && !it.value().contains(device.adapterId()))
            {
                qCWarning(scopeGeneralInfo) << qUtf8Printable(
                  QString("ProjectFileHandler: device %1 is not listed by its adapter '%2' but by '%3' - moved to '%3'")
                    .arg(devId)
                    .arg(device.adapterId(), firstClaimant));
                device.setAdapterId(firstClaimant);
            }
        }
    }

    /* Adapter configs hold device IDs as JSON ints, so an ID beyond that range can't be written there */
    const deviceId_t maxConfigDeviceId = static_cast<deviceId_t>(std::numeric_limits<int>::max());
    for (auto it = devices.cbegin(), end = devices.cend(); it != end; ++it)
    {
        const deviceId_t devId = it.key();
        const QString owner = it.value().adapterId();
        if (!claims.contains(devId) && listingAdapters.contains(owner) && devId <= maxConfigDeviceId)
        {
            QJsonObject& settings = adapters[listingAdapters.value(owner)].settings;
            QJsonArray configDevices = settings.value("devices").toArray();
            configDevices.append(QJsonObject{ { "id", static_cast<int>(devId) } });
            settings["devices"] = configDevices;
            qCWarning(scopeGeneralInfo) << qUtf8Printable(
              QString("ProjectFileHandler: device %1 is missing from the config of adapter '%2' - added with defaults")
                .arg(devId)
                .arg(owner));
        }
    }
}

/*! \brief Collect, for every device ID, the adapters whose config lists it.
 * \param adapters         The adapters section of the project file.
 * \param listingAdapters  Adapter type -> index into \a adapters, for adapters whose settings hold a "devices" array.
 * \return Per device ID, the adapter types listing it in alphabetical order. Entries without a valid ID are ignored.
 */
QMap<deviceId_t, QStringList> ProjectFileHandler::collectDeviceClaims(
  const QList<ProjectFileData::AdapterFileSettings>& adapters, const QMap<QString, qsizetype>& listingAdapters)
{
    QMap<deviceId_t, QStringList> claims;
    for (auto it = listingAdapters.cbegin(), end = listingAdapters.cend(); it != end; ++it)
    {
        const QJsonArray configDevices = adapters[it.value()].settings.value("devices").toArray();
        for (const auto& configDevice : configDevices)
        {
            const int id = configDevice.toObject().value("id").toInt(-1);
            if (id < 0)
            {
                continue;
            }

            QStringList& claimants = claims[static_cast<deviceId_t>(id)];
            if (!claimants.contains(it.key()))
            {
                claimants.append(it.key());
            }
        }
    }
    return claims;
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
