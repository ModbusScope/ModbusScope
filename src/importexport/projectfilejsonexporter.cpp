
#include "projectfilejsonexporter.h"

#include "importexport/projectfiledefinitions.h"
#include "models/connectiontypes.h"
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
 */
void ProjectFileJsonExporter::exportProjectFile(const QString& projectFile)
{
    QJsonObject root;
    root[ProjectFileDefinitions::cVersionKey] = static_cast<int>(ProjectFileDefinitions::cCurrentJsonVersion);
    root[ProjectFileDefinitions::cAdaptersKey] = createAdaptersArray();
    root[ProjectFileDefinitions::cDevicesJsonKey] = createDevicesArray();
    root[ProjectFileDefinitions::cLogTag] = createLogObject();
    root[ProjectFileDefinitions::cScopeTag] = createScopeArray();
    root[ProjectFileDefinitions::cViewTag] = createViewObject();

    QJsonDocument doc(root);

    QSaveFile file(projectFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        file.write(doc.toJson(QJsonDocument::Indented));
        if (!file.commit())
        {
            Util::showError(tr("Export settings to file (%1) failed").arg(projectFile));
        }
    }
    else
    {
        Util::showError(tr("Export settings to file (%1) failed").arg(projectFile));
    }
}

/*!
 * \brief Build the top-level "adapters" array.
 *
 * \note This implementation only emits the modbus adapter. Non-modbus adapter
 *       data is not preserved on save. This is a known transitional limitation
 *       that will be resolved when adapter-specific storage is moved out of
 *       SettingsModel in a future refactoring.
 *
 * For the modbus adapter, the settings object contains:
 *  - connections[]: one entry per enabled/disabled connection in SettingsModel
 *  - devices[]: adapter-specific device fields (slaveId, connectionId, etc.)
 */
QJsonArray ProjectFileJsonExporter::createAdaptersArray()
{
    QJsonArray connectionsArray;
    for (quint8 i = 0u; i < ConnectionTypes::ID_CNT; i++)
    {
        auto* connData = _pSettingsModel->connectionSettings(i);
        QJsonObject connObj;
        connObj[ProjectFileDefinitions::cIdJsonKey] = static_cast<int>(i);

        if (connData->connectionType() == ConnectionTypes::TYPE_TCP)
        {
            connObj[ProjectFileDefinitions::cConnectionTypeJsonKey] = QString("tcp");
        }
        else
        {
            connObj[ProjectFileDefinitions::cConnectionTypeJsonKey] = QString("serial");
        }

        connObj[ProjectFileDefinitions::cIpTag] = connData->ipAddress();
        connObj[ProjectFileDefinitions::cPortTag] = static_cast<int>(connData->port());
        connObj[ProjectFileDefinitions::cPortNameTag] = connData->portName();
        connObj[ProjectFileDefinitions::cBaudrateTag] = static_cast<int>(connData->baudrate());
        connObj[ProjectFileDefinitions::cParityTag] = static_cast<int>(connData->parity());
        connObj[ProjectFileDefinitions::cStopBitsTag] = static_cast<int>(connData->stopbits());
        connObj[ProjectFileDefinitions::cDataBitsTag] = static_cast<int>(connData->databits());
        connObj[ProjectFileDefinitions::cTimeoutTag] = static_cast<int>(connData->timeout());
        connObj[ProjectFileDefinitions::cPersistentConnectionTag] = connData->persistentConnection();
        connObj[ProjectFileDefinitions::cConnectionEnabledTag] = _pSettingsModel->connectionState(i);

        connectionsArray.append(connObj);
    }

    QJsonArray adapterDevicesArray;
    QList<deviceId_t> deviceList = _pSettingsModel->deviceList();
    int adapterDeviceIndex = 0;
    for (const deviceId_t& deviceId : std::as_const(deviceList))
    {
        Device* device = _pSettingsModel->deviceSettings(deviceId);
        QJsonObject devObj;
        devObj[ProjectFileDefinitions::cIdJsonKey] = adapterDeviceIndex;
        devObj[ProjectFileDefinitions::cConnectionIdTag] = static_cast<int>(device->connectionId());
        devObj[ProjectFileDefinitions::cSlaveIdTag] = static_cast<int>(device->slaveId());
        devObj[ProjectFileDefinitions::cConsecutiveMaxTag] = static_cast<int>(device->consecutiveMax());
        devObj[ProjectFileDefinitions::cInt32LittleEndianTag] = device->int32LittleEndian();
        adapterDevicesArray.append(devObj);
        adapterDeviceIndex++;
    }

    QJsonObject modbusSettings;
    modbusSettings[ProjectFileDefinitions::cConnectionsJsonKey] = connectionsArray;
    modbusSettings[ProjectFileDefinitions::cDevicesJsonKey] = adapterDevicesArray;

    QJsonObject modbusAdapter;
    modbusAdapter[ProjectFileDefinitions::cAdapterTypeKey] = QString("modbus");
    modbusAdapter[ProjectFileDefinitions::cAdapterSettingsKey] = modbusSettings;

    QJsonArray adaptersArray;
    adaptersArray.append(modbusAdapter);
    return adaptersArray;
}

/*!
 * \brief Build the top-level "devices" array (generic device info only).
 */
QJsonArray ProjectFileJsonExporter::createDevicesArray()
{
    QJsonArray devicesArray;
    QList<deviceId_t> deviceList = _pSettingsModel->deviceList();

    for (const deviceId_t& deviceId : std::as_const(deviceList))
    {
        Device* device = _pSettingsModel->deviceSettings(deviceId);
        QJsonObject adapterRef;
        adapterRef[ProjectFileDefinitions::cAdapterTypeKey] = QString("modbus");

        QJsonObject devObj;
        devObj[ProjectFileDefinitions::cIdJsonKey] = static_cast<int>(deviceId);
        devObj[ProjectFileDefinitions::cAdapterIdKey] = 0;
        devObj[ProjectFileDefinitions::cDeviceNameTag] = device->name();
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
