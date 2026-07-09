#include "tst_guistatecontroller.h"

#include "models/dataparsermodel.h"
#include "models/guimodel.h"
#include "models/settingsmodel.h"

#include <QAction>
#include <QJsonObject>
#include <QTest>

using GuiState = GuiModel::GuiState;

void TestGuiStateController::init()
{
    _pGuiModel = new GuiModel(this);
    _pSettingsModel = new SettingsModel(this);
    _pDataParserModel = new DataParserModel(this);

    _pStart = new QAction(this);
    _pStop = new QAction(this);
    _pSettings = new QAction(this);
    _pRegisterSettings = new QAction(this);
    _pOpenDataFile = new QAction(this);
    _pImportFromMbcFile = new QAction(this);
    _pOpenProjectFile = new QAction(this);
    _pSaveDataFile = new QAction(this);
    _pExportImage = new QAction(this);
    _pSaveProjectFileAs = new QAction(this);
    _pSaveProjectFile = new QAction(this);
    _pReloadProjectFile = new QAction(this);
    _pClearData = new QAction(this);

    const GuiStateActions actions{
        .pStart = _pStart,
        .pStop = _pStop,
        .pSettings = _pSettings,
        .pRegisterSettings = _pRegisterSettings,
        .pOpenDataFile = _pOpenDataFile,
        .pImportFromMbcFile = _pImportFromMbcFile,
        .pOpenProjectFile = _pOpenProjectFile,
        .pSaveDataFile = _pSaveDataFile,
        .pExportImage = _pExportImage,
        .pSaveProjectFileAs = _pSaveProjectFileAs,
        .pSaveProjectFile = _pSaveProjectFile,
        .pReloadProjectFile = _pReloadProjectFile,
        .pClearData = _pClearData,
    };

    _pController = new GuiStateController(_pGuiModel, _pSettingsModel, _pDataParserModel, actions, this);
}

void TestGuiStateController::cleanup()
{
    delete _pController;
    _pController = nullptr;

    delete _pGuiModel;
    _pGuiModel = nullptr;
    delete _pSettingsModel;
    _pSettingsModel = nullptr;
    delete _pDataParserModel;
    _pDataParserModel = nullptr;

    _pStart = nullptr;
    _pStop = nullptr;
    _pSettings = nullptr;
    _pRegisterSettings = nullptr;
    _pOpenDataFile = nullptr;
    _pImportFromMbcFile = nullptr;
    _pOpenProjectFile = nullptr;
    _pSaveDataFile = nullptr;
    _pExportImage = nullptr;
    _pSaveProjectFileAs = nullptr;
    _pSaveProjectFile = nullptr;
    _pReloadProjectFile = nullptr;
    _pClearData = nullptr;
}

void TestGuiStateController::initState()
{
    _pGuiModel->setGuiState(GuiState::INIT);

    QVERIFY(!_pStop->isEnabled());
    QVERIFY(_pSettings->isEnabled());
    QVERIFY(_pRegisterSettings->isEnabled());
    QVERIFY(_pStart->isEnabled());
    QVERIFY(_pOpenDataFile->isEnabled());
    QVERIFY(_pOpenProjectFile->isEnabled());
    QVERIFY(!_pSaveDataFile->isEnabled());
    QVERIFY(!_pExportImage->isEnabled());
    QVERIFY(_pSaveProjectFileAs->isEnabled());
    QVERIFY(_pClearData->isEnabled());

    QVERIFY(_pGuiModel->projectFilePath().isEmpty());
}

void TestGuiStateController::startedState()
{
    _pGuiModel->setGuiState(GuiState::STARTED);

    QVERIFY(_pStop->isEnabled());
    QVERIFY(!_pSettings->isEnabled());
    QVERIFY(!_pRegisterSettings->isEnabled());
    QVERIFY(!_pStart->isEnabled());
    QVERIFY(!_pOpenDataFile->isEnabled());
    QVERIFY(!_pImportFromMbcFile->isEnabled());
    QVERIFY(!_pOpenProjectFile->isEnabled());
    QVERIFY(!_pSaveDataFile->isEnabled());
    QVERIFY(!_pSaveProjectFileAs->isEnabled());
    QVERIFY(!_pSaveProjectFile->isEnabled());
    QVERIFY(!_pReloadProjectFile->isEnabled());
    QVERIFY(_pExportImage->isEnabled());
    QVERIFY(_pClearData->isEnabled());
}

void TestGuiStateController::stoppedStateNoProject()
{
    _pGuiModel->setGuiState(GuiState::STOPPED);

    QVERIFY(!_pStop->isEnabled());
    QVERIFY(_pSettings->isEnabled());
    QVERIFY(_pRegisterSettings->isEnabled());
    QVERIFY(_pStart->isEnabled());
    QVERIFY(_pOpenDataFile->isEnabled());
    QVERIFY(_pOpenProjectFile->isEnabled());
    QVERIFY(_pSaveDataFile->isEnabled());
    QVERIFY(_pSaveProjectFileAs->isEnabled());
    QVERIFY(_pExportImage->isEnabled());
    QVERIFY(_pClearData->isEnabled());

    QVERIFY(!_pReloadProjectFile->isEnabled());
    QVERIFY(!_pSaveProjectFile->isEnabled());
}

void TestGuiStateController::stoppedStateWithProject()
{
    _pGuiModel->setProjectFilePath("/tmp/project.mbs");

    _pGuiModel->setGuiState(GuiState::STOPPED);

    QVERIFY(_pReloadProjectFile->isEnabled());
    QVERIFY(_pSaveProjectFile->isEnabled());
}

void TestGuiStateController::dataLoadedState()
{
    _pDataParserModel->setDataFilePath("/tmp/data.csv");

    _pGuiModel->setGuiState(GuiState::DATA_LOADED);

    QVERIFY(!_pStop->isEnabled());
    QVERIFY(_pSettings->isEnabled());
    QVERIFY(_pStart->isEnabled());
    QVERIFY(_pOpenDataFile->isEnabled());
    QVERIFY(_pOpenProjectFile->isEnabled());

    QVERIFY(!_pSaveDataFile->isEnabled());
    QVERIFY(!_pSaveProjectFileAs->isEnabled());
    QVERIFY(!_pSaveProjectFile->isEnabled());
    QVERIFY(_pExportImage->isEnabled());
    QVERIFY(!_pClearData->isEnabled());
    QVERIFY(!_pReloadProjectFile->isEnabled());

    QCOMPARE(_pGuiModel->windowTitle(), QStringLiteral("ModbusScope - data.csv"));
}

void TestGuiStateController::projectFileLoadedTogglesActions()
{
    _pGuiModel->setProjectFilePath("/tmp/project.mbs");

    QVERIFY(_pReloadProjectFile->isEnabled());
    QVERIFY(_pSaveProjectFile->isEnabled());
    QVERIFY(_pGuiModel->windowTitle().contains("project.mbs"));

    _pGuiModel->setProjectFilePath("");

    QVERIFY(!_pReloadProjectFile->isEnabled());
    QVERIFY(!_pSaveProjectFile->isEnabled());
    QVERIFY(!_pGuiModel->windowTitle().contains("project.mbs"));
}

void TestGuiStateController::initStateDisablesProjectActions()
{
    // Reach STOPPED with a loaded project so reload/save-project are enabled ...
    _pGuiModel->setProjectFilePath("/tmp/project.mbs");
    _pGuiModel->setGuiState(GuiState::STOPPED);
    QVERIFY(_pReloadProjectFile->isEnabled());
    QVERIFY(_pSaveProjectFile->isEnabled());

    // ... then transition straight to INIT: the state handler must disable
    // reload/save-project itself rather than relying on the reentrant
    // projectFilePathChanged emission triggered by clearing the path.
    _pGuiModel->setGuiState(GuiState::INIT);

    QVERIFY(!_pReloadProjectFile->isEnabled());
    QVERIFY(!_pSaveProjectFile->isEnabled());
}

void TestGuiStateController::mbcCompatibilityRespected()
{
    _pGuiModel->setGuiState(GuiState::INIT);
    QVERIFY(!_pImportFromMbcFile->isEnabled());

    QJsonObject describeResult;
    QJsonObject capabilities;
    capabilities["mbcCompatible"] = true;
    describeResult["capabilities"] = capabilities;
    _pSettingsModel->updateAdapterFromDescribe("modbus", describeResult);

    QVERIFY(_pImportFromMbcFile->isEnabled());
}

QTEST_MAIN(TestGuiStateController)
