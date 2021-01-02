
#include <QFile>
#include <QFileDialog>

#include "util.h"
#include "projectfileexporter.h"
#include "updateregisteroperations.h"

#include "projectfilehandler.h"

ProjectFileHandler::ProjectFileHandler(GuiModel* pGuiModel, SettingsModel* pSettingsModel, GraphDataModel* pGraphDataModel) : QObject(nullptr)
{
    _pGuiModel = pGuiModel;
    _pSettingsModel = pSettingsModel;
    _pGraphDataModel = pGraphDataModel;
}

void ProjectFileHandler::loadProjectFile(QString projectFilePath)
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

void ProjectFileHandler::selectSettingsExportFile()
{
    ProjectFileExporter projectFileExporter(_pGuiModel, _pSettingsModel, _pGraphDataModel);
    QString filePath;
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::HideNameFilterDetails, false);
    dialog.setDefaultSuffix("mbs");
    dialog.setWindowTitle(tr("Select mbs file"));
    dialog.setNameFilter(tr("MBS files (*.mbs)"));
    dialog.setDirectory(_pGuiModel->lastDir());

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        _pGuiModel->setLastDir(QFileInfo(filePath).dir().absolutePath());
        projectFileExporter.exportProjectFile(filePath);
    }
}

void ProjectFileHandler::selectProjectSettingFile()
{
    QString filePath;
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setOption(QFileDialog::HideNameFilterDetails, false);
    dialog.setWindowTitle(tr("Select mbs file"));
    dialog.setNameFilter(tr("mbs files (*.mbs)"));
    dialog.setDirectory(_pGuiModel->lastDir());

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        _pGuiModel->setLastDir(QFileInfo(filePath).dir().absolutePath());
        this->loadProjectFile(filePath);
    }
}

void ProjectFileHandler::reloadProjectFile()
{
    this->loadProjectFile(_pGuiModel->projectFilePath());
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
            /* Default to connection 0 */
            connectionId = SettingsModel::CONNECTION_ID_0;
        }

        if (connectionId < SettingsModel::CONNECTION_ID_CNT)
        {
            _pSettingsModel->setConnectionState(connectionId, true);

            if (pProjectSettings->general.connectionSettings[idx].bIp)
            {
                _pSettingsModel->setIpAddress(connectionId, pProjectSettings->general.connectionSettings[idx].ip);
            }

            /* TODO for now: default to TCP */
            _pSettingsModel->setConnectionType(connectionId, SettingsModel::CONNECTION_TYPE_TCP);

            if (pProjectSettings->general.connectionSettings[idx].bPort)
            {
                 _pSettingsModel->setPort(connectionId, pProjectSettings->general.connectionSettings[idx].port);
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
        rowData.setUnsigned(pSettingData->bUnsigned);
        rowData.setBit32(pSettingData->b32Bit);
        rowData.setRegisterAddress(pSettingData->address);
        rowData.setLabel(pSettingData->text);
        rowData.setColor(pSettingData->color);
        rowData.setConnectionId(pSettingData->connectionId);

        if (pSettingData->bExpression)
        {
            rowData.setExpression(pSettingData->expression);
        }
        else
        {
            QString convertedExpression;
            UpdateRegisterOperations::convert(*pSettingData, convertedExpression);

            rowData.setExpression(convertedExpression);
        }

        _pGraphDataModel->add(rowData);
    }

    _pGuiModel->setFrontGraph(-1);
}
