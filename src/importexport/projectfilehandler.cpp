
#include "projectfilehandler.h"

#include "importexport/projectfiledata.h"
#include "importexport/projectfileexporter.h"
#include "importexport/projectfileparser.h"
#include "models/graphdatamodel.h"
#include "models/guimodel.h"
#include "models/settingsmodel.h"
#include "util/fileselectionhelper.h"
#include "util/util.h"

#include <QFile>
#include <QFileDialog>

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

        GeneralError parseErr = fileParser.parseFile(projectFileContents, &loadedSettings);
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
        ProjectFileExporter projectFileExporter(_pGuiModel, _pSettingsModel, _pGraphDataModel);
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
    const int connCnt = pProjectSettings->general.connectionSettings.size();

    for(int idx = 0; idx < connCnt; idx++)
    {
        quint8 connectionId;

        if (pProjectSettings->general.connectionSettings[idx].bConnectionId)
        {
            connectionId = pProjectSettings->general.connectionSettings[idx].connectionId;
        }
        else
        {
            /* Default to connection 1 */
            connectionId = ConnectionId::ID_1;
        }

        if (connectionId < ConnectionId::ID_CNT)
        {
            auto connData = _pSettingsModel->connectionSettings(connectionId);

            _pSettingsModel->setConnectionState(connectionId, pProjectSettings->general.connectionSettings[idx].bConnectionState);

            if (pProjectSettings->general.connectionSettings[idx].bConnectionType
                && pProjectSettings->general.connectionSettings[idx].connectionType.toLower() == "serial"
                )
            {
                connData->setConnectionType(Connection::TYPE_SERIAL);
            }
            else
            {
                connData->setConnectionType(Connection::TYPE_TCP);
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

            if (pProjectSettings->general.connectionSettings[idx].bSlaveId)
            {
                connData->setSlaveId(pProjectSettings->general.connectionSettings[idx].slaveId);
            }

            if (pProjectSettings->general.connectionSettings[idx].bTimeout)
            {
                connData->setTimeout(pProjectSettings->general.connectionSettings[idx].timeout);
            }

            if (pProjectSettings->general.connectionSettings[idx].bConsecutiveMax)
            {
                connData->setConsecutiveMax(pProjectSettings->general.connectionSettings[idx].consecutiveMax);
            }

            connData->setInt32LittleEndian(pProjectSettings->general.connectionSettings[idx].bInt32LittleEndian);

            connData->setPersistentConnection(pProjectSettings->general.connectionSettings[idx].bPersistentConnection);
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
