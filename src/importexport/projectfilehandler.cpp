
#include "projectfilehandler.h"

#include "importexport/projectfiledata.h"
#include "importexport/projectfiledefinitions.h"
#include "importexport/projectfileexporter.h"
#include "importexport/projectfilejsonexporter.h"
#include "importexport/projectfilejsonparser.h"
#include "importexport/projectfileparser.h"
#include "models/graphdatamodel.h"
#include "models/guimodel.h"
#include "models/settingsmodel.h"
#include "util/fileselectionhelper.h"
#include "util/util.h"

#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonObject>

using connectionId_t = ConnectionTypes::connectionId_t;

ProjectFileHandler::ProjectFileHandler(GuiModel* pGuiModel, SettingsModel* pSettingsModel, GraphDataModel* pGraphDataModel) : QObject(nullptr)
{
    _pGuiModel = pGuiModel;
    _pSettingsModel = pSettingsModel;
    _pGraphDataModel = pGraphDataModel;
}

void ProjectFileHandler::openProjectFile(QString projectFilePath)
{
    ProjectFileParser fileParser;
    ProjectFileData::ProjectSettings loadedSettings;
    QFile file(projectFilePath);

    /* If we can't open it, let's show an error message. */
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&file);
        QString projectFileContents = in.readAll();

        GeneralError parseErr;
        if (projectFileContents.trimmed().startsWith('{'))
        {
            ProjectFileJsonParser jsonParser;
            parseErr = jsonParser.parseFile(projectFileContents, &loadedSettings);
        }
        else
        {
            parseErr = fileParser.parseFile(projectFileContents, &loadedSettings);
        }
        if (parseErr.result())
        {
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
    FileSelectionHelper::configureFileDialog(&dialog,
                                             FileSelectionHelper::DIALOG_TYPE_SAVE,
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
        projectFileExporter.exportProjectFile(projectFile);
    }
}

void ProjectFileHandler::selectProjectOpenFile()
{
    QFileDialog dialog;
    FileSelectionHelper::configureFileDialog(&dialog,
                                             FileSelectionHelper::DIALOG_TYPE_OPEN,
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

void ProjectFileHandler::updateProjectSetting(ProjectFileData::ProjectSettings * pProjectSettings)
{
    if (!pProjectSettings->general.adapterList.isEmpty())
    {
        /* JSON format path: extract modbus-specific settings from the adapter settings blob.
         * This is transitional code — Connection/Device in SettingsModel will be removed later. */

        /* Clear all devices once before processing any adapter */
        _pSettingsModel->removeAllDevice();
        bool bAnyDeviceAdded = false;

        const QList<ProjectFileData::AdapterFileSettings>& adapterList = pProjectSettings->general.adapterList;
        for (int adapterIdx = 0; adapterIdx < adapterList.size(); adapterIdx++)
        {
            const ProjectFileData::AdapterFileSettings& adapter = adapterList[adapterIdx];

            if (adapter.type.compare("modbus", Qt::CaseInsensitive) != 0)
            {
                continue;
            }

            /* Apply connection settings from adapter blob */
            if (adapter.settings.contains(ProjectFileDefinitions::cConnectionsJsonKey))
            {
                QJsonArray connsArray = adapter.settings[ProjectFileDefinitions::cConnectionsJsonKey].toArray();
                for (const QJsonValue& connVal : std::as_const(connsArray))
                {
                    if (!connVal.isObject())
                    {
                        continue;
                    }
                    QJsonObject connObj = connVal.toObject();

                    const int rawConnectionId = connObj[ProjectFileDefinitions::cIdJsonKey].toInt(-1);
                    if (rawConnectionId < ConnectionTypes::ID_1 || rawConnectionId >= ConnectionTypes::ID_CNT)
                    {
                        continue;
                    }
                    connectionId_t connectionId = static_cast<connectionId_t>(rawConnectionId);

                    auto connData = _pSettingsModel->connectionSettings(connectionId);
                    bool bEnabled = connObj[ProjectFileDefinitions::cConnectionEnabledTag].toBool(true);
                    _pSettingsModel->setConnectionState(connectionId, bEnabled);

                    QString connType = connObj[ProjectFileDefinitions::cConnectionTypeJsonKey].toString().toLower();
                    if (connType == "serial")
                    {
                        connData->setConnectionType(ConnectionTypes::TYPE_SERIAL);
                    }
                    else
                    {
                        connData->setConnectionType(ConnectionTypes::TYPE_TCP);
                    }

                    if (connObj.contains(ProjectFileDefinitions::cIpTag))
                    {
                        connData->setIpAddress(connObj[ProjectFileDefinitions::cIpTag].toString());
                    }

                    if (connObj.contains(ProjectFileDefinitions::cPortTag))
                    {
                        connData->setPort(static_cast<quint16>(connObj[ProjectFileDefinitions::cPortTag].toInt()));
                    }

                    if (connObj.contains(ProjectFileDefinitions::cPortNameTag))
                    {
                        connData->setPortName(connObj[ProjectFileDefinitions::cPortNameTag].toString());
                    }

                    const quint32 detectedBaud = static_cast<quint32>(connObj[ProjectFileDefinitions::cBaudrateTag].toInt(0));
                    if (
                        detectedBaud == QSerialPort::Baud1200
                        || detectedBaud == QSerialPort::Baud2400
                        || detectedBaud == QSerialPort::Baud4800
                        || detectedBaud == QSerialPort::Baud9600
                        || detectedBaud == QSerialPort::Baud19200
                        || detectedBaud == QSerialPort::Baud38400
                        || detectedBaud == QSerialPort::Baud57600
                        || detectedBaud == QSerialPort::Baud115200
                    )
                    {
                        connData->setBaudrate(static_cast<QSerialPort::BaudRate>(detectedBaud));
                    }

                    const quint32 detectedParity = static_cast<quint32>(connObj[ProjectFileDefinitions::cParityTag].toInt(0));
                    if (
                        detectedParity == QSerialPort::NoParity
                        || detectedParity == QSerialPort::EvenParity
                        || detectedParity == QSerialPort::OddParity
                    )
                    {
                        connData->setParity(static_cast<QSerialPort::Parity>(detectedParity));
                    }

                    const quint32 detectedStopBits = static_cast<quint32>(connObj[ProjectFileDefinitions::cStopBitsTag].toInt(0));
                    if (
                        detectedStopBits == QSerialPort::OneStop
                        || detectedStopBits == QSerialPort::OneAndHalfStop
                        || detectedStopBits == QSerialPort::TwoStop
                    )
                    {
                        connData->setStopbits(static_cast<QSerialPort::StopBits>(detectedStopBits));
                    }

                    const quint32 detectedDataBits = static_cast<quint32>(connObj[ProjectFileDefinitions::cDataBitsTag].toInt(0));
                    if (
                        detectedDataBits == QSerialPort::Data5
                        || detectedDataBits == QSerialPort::Data6
                        || detectedDataBits == QSerialPort::Data7
                        || detectedDataBits == QSerialPort::Data8
                    )
                    {
                        connData->setDatabits(static_cast<QSerialPort::DataBits>(detectedDataBits));
                    }

                    if (connObj.contains(ProjectFileDefinitions::cTimeoutTag))
                    {
                        connData->setTimeout(static_cast<quint32>(connObj[ProjectFileDefinitions::cTimeoutTag].toInt()));
                    }

                    connData->setPersistentConnection(
                      connObj[ProjectFileDefinitions::cPersistentConnectionTag].toBool(true));
                }
            }

            /* Collect the generic devices that belong to this adapter */
            QList<ProjectFileData::DeviceSettings> adapterGenericDevices;
            for (const ProjectFileData::DeviceSettings& dev : std::as_const(pProjectSettings->general.deviceSettings))
            {
                if (dev.adapterId == static_cast<quint32>(adapterIdx))
                {
                    adapterGenericDevices.append(dev);
                }
            }

            QJsonArray adapterDevices;
            if (adapter.settings.contains(ProjectFileDefinitions::cDevicesJsonKey))
            {
                adapterDevices = adapter.settings[ProjectFileDefinitions::cDevicesJsonKey].toArray();
            }

            /* Match adapter-level device entries with their generic counterparts by position */
            int matchCount = qMin(adapterDevices.size(), adapterGenericDevices.size());
            for (int idx = 0; idx < matchCount; idx++)
            {
                deviceId_t deviceId = Device::cFirstDeviceId;
                if (adapterGenericDevices[idx].bDeviceId)
                {
                    deviceId = adapterGenericDevices[idx].deviceId;
                }
                _pSettingsModel->addDevice(deviceId);
                bAnyDeviceAdded = true;

                auto deviceData = _pSettingsModel->deviceSettings(deviceId);

                if (adapterGenericDevices[idx].bName)
                {
                    deviceData->setName(adapterGenericDevices[idx].name);
                }

                QJsonObject adapterDev = adapterDevices[idx].toObject();

                const int rawConnId = adapterDev[ProjectFileDefinitions::cConnectionIdTag].toInt(ConnectionTypes::ID_1);
                const connectionId_t connectionId =
                  (rawConnId >= ConnectionTypes::ID_1 && rawConnId < ConnectionTypes::ID_CNT)
                    ? static_cast<connectionId_t>(rawConnId)
                    : static_cast<connectionId_t>(ConnectionTypes::ID_1);
                deviceData->setConnectionId(connectionId);

                if (adapterDev.contains(ProjectFileDefinitions::cSlaveIdTag))
                {
                    deviceData->setSlaveId(static_cast<quint8>(adapterDev[ProjectFileDefinitions::cSlaveIdTag].toInt()));
                }

                if (adapterDev.contains(ProjectFileDefinitions::cConsecutiveMaxTag))
                {
                    deviceData->setConsecutiveMax(
                      static_cast<quint8>(adapterDev[ProjectFileDefinitions::cConsecutiveMaxTag].toInt()));
                }

                deviceData->setInt32LittleEndian(
                  adapterDev[ProjectFileDefinitions::cInt32LittleEndianTag].toBool(true));
            }
        }

        /* Ensure at least one device exists when the file defines none */
        if (!bAnyDeviceAdded)
        {
            _pSettingsModel->addDevice(Device::cFirstDeviceId);
        }
    }
    else
    {
        /* XML format path (legacy) */
        const int connCnt = pProjectSettings->general.connectionSettings.size();

        for (int idx = 0; idx < connCnt; idx++)
        {
            connectionId_t connectionId;

            if (pProjectSettings->general.connectionSettings[idx].bConnectionId)
            {
                connectionId = pProjectSettings->general.connectionSettings[idx].connectionId;
            }
            else
            {
                /* Default to connection 1 */
                connectionId = ConnectionTypes::ID_1;
            }

            if (connectionId < ConnectionTypes::ID_CNT)
            {
                auto connData = _pSettingsModel->connectionSettings(connectionId);

                _pSettingsModel->setConnectionState(connectionId, pProjectSettings->general.connectionSettings[idx].bConnectionState);

                if (pProjectSettings->general.connectionSettings[idx].bConnectionType
                    && pProjectSettings->general.connectionSettings[idx].connectionType.toLower() == "serial"
                    )
                {
                    connData->setConnectionType(ConnectionTypes::TYPE_SERIAL);
                }
                else
                {
                    connData->setConnectionType(ConnectionTypes::TYPE_TCP);
                }

                if (pProjectSettings->general.connectionSettings[idx].bIp)
                {
                    connData->setIpAddress(pProjectSettings->general.connectionSettings[idx].ip);
                }

                if (pProjectSettings->general.connectionSettings[idx].bPort)
                {
                    connData->setPort(pProjectSettings->general.connectionSettings[idx].port);
                }

                if (pProjectSettings->general.connectionSettings[idx].bPortName)
                {
                    connData->setPortName(pProjectSettings->general.connectionSettings[idx].portName);
                }

                const quint32 detectedBaud = pProjectSettings->general.connectionSettings[idx].baudrate;
                if (pProjectSettings->general.connectionSettings[idx].bBaudrate)
                {
                    if (
                        detectedBaud == QSerialPort::Baud1200
                        || detectedBaud == QSerialPort::Baud2400
                        || detectedBaud == QSerialPort::Baud4800
                        || detectedBaud == QSerialPort::Baud9600
                        || detectedBaud == QSerialPort::Baud19200
                        || detectedBaud == QSerialPort::Baud38400
                        || detectedBaud == QSerialPort::Baud57600
                        || detectedBaud == QSerialPort::Baud115200
                    )
                    {
                        connData->setBaudrate(static_cast<QSerialPort::BaudRate>(detectedBaud));
                    }
                }

                const quint32 detectedParity = pProjectSettings->general.connectionSettings[idx].parity;
                if (pProjectSettings->general.connectionSettings[idx].bParity)
                {
                    if (
                        detectedParity == QSerialPort::NoParity
                        || detectedParity == QSerialPort::EvenParity
                        || detectedParity == QSerialPort::OddParity
                    )
                    {
                        connData->setParity(static_cast<QSerialPort::Parity>(detectedParity));
                    }
                }

                const quint32 detectedStopBits = pProjectSettings->general.connectionSettings[idx].stopbits;
                if (pProjectSettings->general.connectionSettings[idx].bStopbits)
                {
                    if (
                        detectedStopBits == QSerialPort::OneStop
                        || detectedStopBits == QSerialPort::OneAndHalfStop
                        || detectedStopBits == QSerialPort::TwoStop
                    )
                    {
                        connData->setStopbits(static_cast<QSerialPort::StopBits>(detectedStopBits));
                    }
                }

                const quint32 detectedDataBits = pProjectSettings->general.connectionSettings[idx].databits;
                if (pProjectSettings->general.connectionSettings[idx].bDatabits)
                {
                    if (
                        detectedDataBits == QSerialPort::Data5
                        || detectedDataBits == QSerialPort::Data6
                        || detectedDataBits == QSerialPort::Data7
                        || detectedDataBits == QSerialPort::Data8
                    )
                    {
                        connData->setDatabits(static_cast<QSerialPort::DataBits>(detectedDataBits));
                    }
                }

                if (pProjectSettings->general.connectionSettings[idx].bTimeout)
                {
                    connData->setTimeout(pProjectSettings->general.connectionSettings[idx].timeout);
                }

                connData->setPersistentConnection(pProjectSettings->general.connectionSettings[idx].bPersistentConnection);
            }
        }

        _pSettingsModel->removeAllDevice();

        const int deviceCnt = pProjectSettings->general.deviceSettings.size();
        if (deviceCnt == 0)
        {
            deviceId_t deviceId = Device::cFirstDeviceId; /* Default to first device */
            _pSettingsModel->addDevice(deviceId);
        }

        for (int idx = 0; idx < deviceCnt; idx++)
        {
            deviceId_t deviceId = Device::cFirstDeviceId; /* Default to first device */

            if (pProjectSettings->general.deviceSettings[idx].bDeviceId)
            {
                deviceId = pProjectSettings->general.deviceSettings[idx].deviceId;
            }
            _pSettingsModel->addDevice(deviceId);

            auto deviceData = _pSettingsModel->deviceSettings(deviceId);

            connectionId_t connectionId = ConnectionTypes::ID_1; /* Default to connection 1 */
            if (pProjectSettings->general.deviceSettings[idx].bConnectionId)
            {
                connectionId = pProjectSettings->general.deviceSettings[idx].connectionId;
            }
            deviceData->setConnectionId(connectionId);

            if (pProjectSettings->general.deviceSettings[idx].bName)
            {
                deviceData->setName(pProjectSettings->general.deviceSettings[idx].name);
            }

            if (pProjectSettings->general.deviceSettings[idx].bSlaveId)
            {
                deviceData->setSlaveId(pProjectSettings->general.deviceSettings[idx].slaveId);
            }

            if (pProjectSettings->general.deviceSettings[idx].bConsecutiveMax)
            {
                deviceData->setConsecutiveMax(pProjectSettings->general.deviceSettings[idx].consecutiveMax);
            }

            deviceData->setInt32LittleEndian(pProjectSettings->general.deviceSettings[idx].bInt32LittleEndian);
        }
    }

    if (pProjectSettings->general.logSettings.bPollTime)
    {
        _pSettingsModel->setPollTime(pProjectSettings->general.logSettings.pollTime);
    }

    _pSettingsModel->setAbsoluteTimes(pProjectSettings->general.logSettings.bAbsoluteTimes);

    _pSettingsModel->setWriteDuringLog(pProjectSettings->general.logSettings.bLogToFile);
    if (pProjectSettings->general.logSettings.bLogToFileFile)
    {
        _pSettingsModel->setWriteDuringLogFile(pProjectSettings->general.logSettings.logFile);
    }
    else
    {
         _pSettingsModel->setWriteDuringLogFileToDefault();
    }

    if (pProjectSettings->view.scaleSettings.xAxis.bSliding)
    {
        _pGuiModel->setxAxisSlidingInterval(static_cast<qint32>(pProjectSettings->view.scaleSettings.xAxis.slidingInterval));
        _pGuiModel->setxAxisScale(AxisMode::SCALE_SLIDING);
    }
    else
    {
        _pGuiModel->setxAxisScale(AxisMode::SCALE_AUTO);
    }

    if (pProjectSettings->view.scaleSettings.yAxis.bMinMax)
    {
        _pGuiModel->setyAxisMin(pProjectSettings->view.scaleSettings.yAxis.scaleMin);
        _pGuiModel->setyAxisMax(pProjectSettings->view.scaleSettings.yAxis.scaleMax);
        _pGuiModel->setyAxisScale(AxisMode::SCALE_MINMAX);
    }
    else if (pProjectSettings->view.scaleSettings.yAxis.bWindowScale)
    {
        _pGuiModel->setyAxisScale(AxisMode::SCALE_WINDOW_AUTO);
    }
    else
    {
        _pGuiModel->setyAxisScale(AxisMode::SCALE_AUTO);
    }

    if (pProjectSettings->view.scaleSettings.y2Axis.bMinMax)
    {
        _pGuiModel->sety2AxisMin(pProjectSettings->view.scaleSettings.y2Axis.scaleMin);
        _pGuiModel->sety2AxisMax(pProjectSettings->view.scaleSettings.y2Axis.scaleMax);
        _pGuiModel->sety2AxisScale(AxisMode::SCALE_MINMAX);
    }
    else if (pProjectSettings->view.scaleSettings.y2Axis.bWindowScale)
    {
        _pGuiModel->sety2AxisScale(AxisMode::SCALE_WINDOW_AUTO);
    }
    else
    {
        _pGuiModel->sety2AxisScale(AxisMode::SCALE_AUTO);
    }

    _pGraphDataModel->clear();
    for (qint32 i = 0; i < pProjectSettings->scope.registerList.size(); i++)
    {
        GraphData rowData;
        ProjectFileData::RegisterSettings const* const pSettingData = &pProjectSettings->scope.registerList[i];

        rowData.setActive(pSettingData->bActive);
        rowData.setLabel(pSettingData->text);
        rowData.setColor(pSettingData->color);
        rowData.setValueAxis(pSettingData->valueAxis == 1 ? GraphData::VALUE_AXIS_SECONDARY : GraphData::VALUE_AXIS_PRIMARY);
        rowData.setExpression(pSettingData->expression);

        _pGraphDataModel->add(rowData);
    }
}
