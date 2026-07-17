#include "dialogs/guistatecontroller.h"

#include "models/dataparsermodel.h"
#include "models/guimodel.h"
#include "models/settingsmodel.h"

#include <QAction>
#include <QFileInfo>

using GuiState = GuiModel::GuiState;

/*!
 * \brief Construct the controller and wire it to the models it reacts to
 * \param pGuiModel GUI state / project-path model
 * \param pSettingsModel Settings model (used for MBC-compatibility of the active adapter)
 * \param pDataParserModel Data-parser model reset on state transitions
 * \param actions Actions whose enabled state this controller manages
 * \param parent Parent QObject
 */
GuiStateController::GuiStateController(GuiModel* pGuiModel,
                                       SettingsModel* pSettingsModel,
                                       DataParserModel* pDataParserModel,
                                       const GuiStateActions& actions,
                                       QObject* parent)
    : QObject(parent),
      _pGuiModel(pGuiModel),
      _pSettingsModel(pSettingsModel),
      _pDataParserModel(pDataParserModel),
      _actions(actions)
{
    connect(_pGuiModel, &GuiModel::guiStateChanged, this, &GuiStateController::updateGuiState);
    connect(_pGuiModel, &GuiModel::projectFilePathChanged, this, &GuiStateController::projectFileLoaded);
    connect(_pSettingsModel, &SettingsModel::adapterDataChanged, this, &GuiStateController::updateGuiState);

    // GuiModel starts in GuiState::INIT without emitting guiStateChanged(), so apply the initial
    // action state here rather than relying on the .ui file's hardcoded defaults to match it.
    updateGuiState();
}

/*!
 * \brief Enable/disable actions and reset related model state for the current GuiState
 */
void GuiStateController::updateGuiState()
{
    if (_pGuiModel->guiState() == GuiState::INIT)
    {
        _actions.pStop->setEnabled(false);
        _actions.pSettings->setEnabled(true);
        _actions.pRegisterSettings->setEnabled(true);
        _actions.pStart->setEnabled(true);
        _actions.pOpenDataFile->setEnabled(true);
        _actions.pImportFromMbcFile->setEnabled(_pSettingsModel->isMbcCompatible());
        _actions.pOpenProjectFile->setEnabled(true);
        _actions.pSaveDataFile->setEnabled(false);
        _actions.pExportImage->setEnabled(false);
        _actions.pSaveProjectFileAs->setEnabled(true);
        _actions.pClearData->setEnabled(true);
        _actions.pReloadProjectFile->setEnabled(false);
        _actions.pSaveProjectFile->setEnabled(false);

        _pDataParserModel->resetSettings();
        _pGuiModel->setProjectFilePath("");

        _pGuiModel->setWindowTitleDetail("");
    }
    else if (_pGuiModel->guiState() == GuiState::STARTED)
    {
        // Communication active
        _actions.pStop->setEnabled(true);
        _actions.pSettings->setEnabled(false);
        _actions.pRegisterSettings->setEnabled(false);
        _actions.pStart->setEnabled(false);
        _actions.pOpenDataFile->setEnabled(false);
        _actions.pImportFromMbcFile->setEnabled(false);
        _actions.pOpenProjectFile->setEnabled(false);
        _actions.pSaveDataFile->setEnabled(false);
        _actions.pSaveProjectFileAs->setEnabled(false);
        _actions.pSaveProjectFile->setEnabled(false);
        _actions.pReloadProjectFile->setEnabled(false);
        _actions.pExportImage->setEnabled(true);
        _actions.pClearData->setEnabled(true);
    }
    else if (_pGuiModel->guiState() == GuiState::STOPPED)
    {
        // Communication not active
        _actions.pStop->setEnabled(false);
        _actions.pSettings->setEnabled(true);
        _actions.pRegisterSettings->setEnabled(true);
        _actions.pStart->setEnabled(true);
        _actions.pOpenDataFile->setEnabled(true);
        _actions.pImportFromMbcFile->setEnabled(_pSettingsModel->isMbcCompatible());
        _actions.pOpenProjectFile->setEnabled(true);
        _actions.pSaveDataFile->setEnabled(true);
        _actions.pSaveProjectFileAs->setEnabled(true);
        _actions.pExportImage->setEnabled(true);
        _actions.pClearData->setEnabled(true);

        _pDataParserModel->resetSettings();

        if (_pGuiModel->projectFilePath().isEmpty())
        {
            _actions.pReloadProjectFile->setEnabled(false);
            _actions.pSaveProjectFile->setEnabled(false);
        }
        else
        {
            _actions.pReloadProjectFile->setEnabled(true);
            _actions.pSaveProjectFile->setEnabled(true);
        }
    }
    else if (_pGuiModel->guiState() == GuiState::DATA_LOADED)
    {
        // Communication not active
        _actions.pStop->setEnabled(false);
        _actions.pSettings->setEnabled(true);
        _actions.pRegisterSettings->setEnabled(true);
        _actions.pStart->setEnabled(true);
        _actions.pOpenDataFile->setEnabled(true);
        _actions.pImportFromMbcFile->setEnabled(_pSettingsModel->isMbcCompatible());
        _actions.pOpenProjectFile->setEnabled(true);

        _actions.pSaveDataFile->setEnabled(false);
        _actions.pSaveProjectFileAs->setEnabled(false);
        _actions.pSaveProjectFile->setEnabled(false);
        _actions.pExportImage->setEnabled(true);
        _actions.pClearData->setEnabled(false);
        _actions.pReloadProjectFile->setEnabled(false);

        _pGuiModel->setWindowTitleDetail(QFileInfo(_pDataParserModel->dataFilePath()).fileName());
    }
}

/*!
 * \brief Toggle reload/save-project actions and the window title detail based on the loaded project path
 */
void GuiStateController::projectFileLoaded()
{
    // if a project file is previously loaded, then it can be reloaded
    if (_pGuiModel->projectFilePath().isEmpty())
    {
        _pGuiModel->setWindowTitleDetail("");
        _actions.pReloadProjectFile->setEnabled(false);
        _actions.pSaveProjectFile->setEnabled(false);
    }
    else
    {
        _pGuiModel->setWindowTitleDetail(QFileInfo(_pGuiModel->projectFilePath()).fileName());
        _actions.pReloadProjectFile->setEnabled(true);
        _actions.pSaveProjectFile->setEnabled(true);
    }
}
