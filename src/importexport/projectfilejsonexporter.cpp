
#include "projectfilejsonexporter.h"

#include "importexport/projectfiledefinitions.h"
#include "models/graphdatamodel.h"
#include "models/guimodel.h"
#include "models/settingsmodel.h"
#include "util/util.h"

#include <QJsonDocument>
#include <QSaveFile>

ProjectFileJsonExporter::ProjectFileJsonExporter(GuiModel* pGuiModel,
                                                 SettingsModel* pSettingsModel,
                                                 GraphDataModel* pGraphDataModel,
                                                 QObject* parent)
    : QObject(parent)
{
    _pGuiModel = pGuiModel;
    _pSettingsModel = pSettingsModel;
    _pGraphDataModel = pGraphDataModel;
}

/*!
 * \brief Serialise current settings and write to \a projectFile.
 * \param projectFile Destination file path.
 * \param adapters    Adapter list to round-trip as-is into the file.
 * \param devices     Generic device list to round-trip as-is into the file.
 */
void ProjectFileJsonExporter::exportProjectFile(const QString& projectFile,
                                                const QList<ProjectFileData::AdapterFileSettings>& adapters,
                                                const QList<ProjectFileData::DeviceSettings>& devices)
{
    QJsonObject root;
    root[ProjectFileDefinitions::cVersionKey] = static_cast<int>(ProjectFileDefinitions::cCurrentJsonVersion);
    root[ProjectFileDefinitions::cAdaptersKey] = createAdaptersArray(adapters);
    root[ProjectFileDefinitions::cDevicesJsonKey] = createDevicesArray(devices);
    root[ProjectFileDefinitions::cLogTag] = createLogObject();
    root[ProjectFileDefinitions::cScopeTag] = createScopeArray();
    root[ProjectFileDefinitions::cViewTag] = createViewObject();

    QJsonDocument doc(root);
    QSaveFile file(projectFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QByteArray jsonData = doc.toJson(QJsonDocument::Indented);
        qint64 bytesWritten = file.write(jsonData);
        if (bytesWritten == jsonData.size())
        {
            if (!file.commit())
            {
                showExportError(projectFile);
            }
        }
        else
        {
            showExportError(projectFile);
        }
    }
    else
    {
        showExportError(projectFile);
    }
}

/*!
 * \brief Build the top-level "adapters" array by round-tripping the loaded adapter data.
 * \param adapters Adapter list parsed from the project file.
 */
QJsonArray ProjectFileJsonExporter::createAdaptersArray(const QList<ProjectFileData::AdapterFileSettings>& adapters)
{
    QJsonArray adaptersArray;
    for (const ProjectFileData::AdapterFileSettings& adapter : std::as_const(adapters))
    {
        QJsonObject adapterObj;
        adapterObj[ProjectFileDefinitions::cAdapterTypeKey] = adapter.type;
        adapterObj[ProjectFileDefinitions::cAdapterSettingsKey] = adapter.settings;
        adaptersArray.append(adapterObj);
    }
    return adaptersArray;
}

/*!
 * \brief Build the top-level "devices" array by round-tripping the loaded generic device data.
 * \param devices Generic device list parsed from the project file.
 */
QJsonArray ProjectFileJsonExporter::createDevicesArray(const QList<ProjectFileData::DeviceSettings>& devices)
{
    QJsonArray devicesArray;
    for (const ProjectFileData::DeviceSettings& dev : std::as_const(devices))
    {
        QJsonObject adapterRef;
        adapterRef[ProjectFileDefinitions::cAdapterTypeKey] = dev.adapterType;

        QJsonObject devObj;
        devObj[ProjectFileDefinitions::cIdJsonKey] = static_cast<int>(dev.deviceId);
        devObj[ProjectFileDefinitions::cAdapterIdKey] = static_cast<int>(dev.adapterId);
        devObj[ProjectFileDefinitions::cDeviceNameTag] = dev.name;
        devObj[ProjectFileDefinitions::cAdapterKey] = adapterRef;
        devicesArray.append(devObj);
    }
    return devicesArray;
}

/*!
 * \brief Build the "log" object.
 */
QJsonObject ProjectFileJsonExporter::createLogObject()
{
    QJsonObject logObj;
    logObj[ProjectFileDefinitions::cPollTimeTag] = static_cast<int>(_pSettingsModel->pollTime());
    logObj[ProjectFileDefinitions::cAbsoluteTimesTag] = _pSettingsModel->absoluteTimes();

    QJsonObject logToFileObj;
    logToFileObj[ProjectFileDefinitions::cEnabledAttribute] = _pSettingsModel->writeDuringLog();

    if (_pSettingsModel->writeDuringLog() && (_pSettingsModel->writeDuringLogFile() != SettingsModel::defaultLogPath()))
    {
        logToFileObj[ProjectFileDefinitions::cFilenameTag] = _pSettingsModel->writeDuringLogFile();
    }

    logObj[ProjectFileDefinitions::cLogToFileTag] = logToFileObj;
    return logObj;
}

/*!
 * \brief Build the "scope" array of register definitions.
 */
QJsonArray ProjectFileJsonExporter::createScopeArray()
{
    QJsonArray scopeArray;

    for (qint32 idx = 0; idx < _pGraphDataModel->size(); idx++)
    {
        QJsonObject regObj;
        regObj[ProjectFileDefinitions::cActiveAttribute] = _pGraphDataModel->isActive(idx);
        regObj[ProjectFileDefinitions::cTextTag] = _pGraphDataModel->label(idx);
        regObj[ProjectFileDefinitions::cExpressionTag] = _pGraphDataModel->expression(idx);
        regObj[ProjectFileDefinitions::cColorTag] = _pGraphDataModel->color(idx).name();
        regObj[ProjectFileDefinitions::cValueAxisTag] = static_cast<int>(_pGraphDataModel->valueAxis(idx));
        scopeArray.append(regObj);
    }

    return scopeArray;
}

/*!
 * \brief Build the "view" object containing scale settings.
 */
QJsonObject ProjectFileJsonExporter::createViewObject()
{
    QJsonObject xaxisObj;
    if (_pGuiModel->xAxisScalingMode() == AxisMode::SCALE_SLIDING)
    {
        xaxisObj[ProjectFileDefinitions::cModeAttribute] = QString(ProjectFileDefinitions::cSlidingValue);
        xaxisObj[ProjectFileDefinitions::cSlidingintervalTag] = static_cast<int>(_pGuiModel->xAxisSlidingSec());
    }
    else
    {
        xaxisObj[ProjectFileDefinitions::cModeAttribute] = QString(ProjectFileDefinitions::cAutoValue);
    }

    auto buildYAxis = [](int axisId, AxisMode::AxisScaleOptions mode, double minVal, double maxVal) -> QJsonObject {
        QJsonObject yaxisObj;
        yaxisObj[ProjectFileDefinitions::cAxisAttribute] = axisId;

        if (mode == AxisMode::SCALE_WINDOW_AUTO)
        {
            yaxisObj[ProjectFileDefinitions::cModeAttribute] = QString(ProjectFileDefinitions::cWindowAutoValue);
        }
        else if (mode == AxisMode::SCALE_MINMAX)
        {
            yaxisObj[ProjectFileDefinitions::cModeAttribute] = QString(ProjectFileDefinitions::cMinmaxValue);
            yaxisObj[ProjectFileDefinitions::cMinTag] = minVal;
            yaxisObj[ProjectFileDefinitions::cMaxTag] = maxVal;
        }
        else
        {
            yaxisObj[ProjectFileDefinitions::cModeAttribute] = QString(ProjectFileDefinitions::cAutoValue);
        }

        return yaxisObj;
    };

    QJsonArray yaxisArray;
    yaxisArray.append(buildYAxis(0, _pGuiModel->yAxisScalingMode(), _pGuiModel->yAxisMin(), _pGuiModel->yAxisMax()));
    yaxisArray.append(buildYAxis(1, _pGuiModel->y2AxisScalingMode(), _pGuiModel->y2AxisMin(), _pGuiModel->y2AxisMax()));

    QJsonObject scaleObj;
    scaleObj[ProjectFileDefinitions::cXaxisTag] = xaxisObj;
    scaleObj[ProjectFileDefinitions::cYaxisTag] = yaxisArray;

    QJsonObject viewObj;
    viewObj[ProjectFileDefinitions::cScaleTag] = scaleObj;
    return viewObj;
}

void ProjectFileJsonExporter::showExportError(const QString& projectFile)
{
    Util::showError(tr("Export settings to file (%1) failed").arg(projectFile));
}
