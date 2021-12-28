
#include <QFile>
#include <QFileDialog>

#include "fileselectionhelper.h"
#include "util.h"
#include "projectfileexporter.h"
#include "updateregisteroperations.h"
#include "updateregisternewexpression.h"

#include "projectfilehandler.h"

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

        if (fileParser.parseFile(projectFileContents, &loadedSettings))
        {
            this->updateProjectSetting(&loadedSettings);

            _pGuiModel->setProjectFilePath(projectFilePath);
            _pGuiModel->setGuiState(GuiModel::STOPPED);
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
        ProjectFileExporter projectFileExporter(_pGuiModel, _pSettingsModel, _pGraphDataModel);
        projectFileExporter.exportProjectFile(selectedFile);
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
            connectionId = SettingsModel::CONNECTION_ID_1;
        }

        if (connectionId < SettingsModel::CONNECTION_ID_CNT)
        {
            _pSettingsModel->setConnectionState(connectionId, pProjectSettings->general.connectionSettings[idx].bConnectionState);

            if (pProjectSettings->general.connectionSettings[idx].bConnectionType
                && pProjectSettings->general.connectionSettings[idx].connectionType.toLower() == "serial"
                )
            {
                _pSettingsModel->setConnectionType(connectionId, SettingsModel::CONNECTION_TYPE_SERIAL);
            }
            else
            {
                _pSettingsModel->setConnectionType(connectionId, SettingsModel::CONNECTION_TYPE_TCP);
            }

            if (pProjectSettings->general.connectionSettings[idx].bIp)
            {
                _pSettingsModel->setIpAddress(connectionId, pProjectSettings->general.connectionSettings[idx].ip);
            }

            if (pProjectSettings->general.connectionSettings[idx].bPort)
            {
                 _pSettingsModel->setPort(connectionId, pProjectSettings->general.connectionSettings[idx].port);
            }

            if (pProjectSettings->general.connectionSettings[idx].bPortName)
            {
                _pSettingsModel->setPortName(connectionId, pProjectSettings->general.connectionSettings[idx].portName);
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
                    _pSettingsModel->setBaudrate(idx, static_cast<QSerialPort::BaudRate>(detectedBaud));
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
                    _pSettingsModel->setParity(idx, static_cast<QSerialPort::Parity>(detectedParity));
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
                    _pSettingsModel->setStopbits(idx, static_cast<QSerialPort::StopBits>(detectedStopBits));
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
                    _pSettingsModel->setDatabits(idx, static_cast<QSerialPort::DataBits>(detectedDataBits));
                }
            }

            if (pProjectSettings->general.connectionSettings[idx].bSlaveId)
            {
                _pSettingsModel->setSlaveId(connectionId, pProjectSettings->general.connectionSettings[idx].slaveId);
            }

            if (pProjectSettings->general.connectionSettings[idx].bTimeout)
            {
                _pSettingsModel->setTimeout(connectionId, pProjectSettings->general.connectionSettings[idx].timeout);
            }

            if (pProjectSettings->general.connectionSettings[idx].bConsecutiveMax)
            {
                _pSettingsModel->setConsecutiveMax(connectionId, pProjectSettings->general.connectionSettings[idx].consecutiveMax);
            }

            _pSettingsModel->setInt32LittleEndian(connectionId, pProjectSettings->general.connectionSettings[idx].bInt32LittleEndian);

            _pSettingsModel->setPersistentConnection(connectionId, pProjectSettings->general.connectionSettings[idx].bPersistentConnection);
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

    if (pProjectSettings->view.scaleSettings.bSliding)
    {
        _pGuiModel->setxAxisSlidingInterval(static_cast<qint32>(pProjectSettings->view.scaleSettings.slidingInterval));
        _pGuiModel->setxAxisScale(AxisMode::SCALE_SLIDING);
    }
    else
    {
        _pGuiModel->setxAxisScale(AxisMode::SCALE_AUTO);
    }

    if (pProjectSettings->view.scaleSettings.bMinMax)
    {
        _pGuiModel->setyAxisMin(pProjectSettings->view.scaleSettings.scaleMin);
        _pGuiModel->setyAxisMax(pProjectSettings->view.scaleSettings.scaleMax);
        _pGuiModel->setyAxisScale(AxisMode::SCALE_MINMAX);
    }
    else if (pProjectSettings->view.scaleSettings.bWindowScale)
    {
        _pGuiModel->setyAxisScale(AxisMode::SCALE_WINDOW_AUTO);
    }
    else
    {
        _pGuiModel->setyAxisScale(AxisMode::SCALE_AUTO);
    }

    _pGraphDataModel->clear();
    for (qint32 i = 0; i < pProjectSettings->scope.registerList.size(); i++)
    {
        GraphData rowData;
        ProjectFileData::RegisterSettings* const pSettingData = &pProjectSettings->scope.registerList[i];

        rowData.setActive(pSettingData->bActive);
        rowData.setLabel(pSettingData->text);
        rowData.setColor(pSettingData->color);

        if (pProjectSettings->dataLevel <= 2)
        {
            if (!pSettingData->bExpression)
            {
                QString convertedExpression;
                UpdateRegisterOperations::convert(*pSettingData, convertedExpression);

                pSettingData->bExpression = true;
                pSettingData->expression = convertedExpression;
            }

            /* Convert to data required for datalevel 3 */
            if (pSettingData->address != 65535)
            {
                /* address tag was present */
                QString convertedExpression;
                UpdateRegisterNewExpression::convert(*pSettingData, convertedExpression);

                pSettingData->expression = convertedExpression;
            }
        }

        rowData.setExpression(pSettingData->expression);

        _pGraphDataModel->add(rowData);
    }

    _pGuiModel->setFrontGraph(-1);
}
