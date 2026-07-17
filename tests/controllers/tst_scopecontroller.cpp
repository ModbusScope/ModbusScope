#include "tst_scopecontroller.h"

#include "controllers/scopecontroller.h"
#include "models/communicationstatsmodel.h"
#include "models/dataparsermodel.h"
#include "models/graphdatamodel.h"
#include "models/guimodel.h"
#include "models/note.h"
#include "models/notemodel.h"
#include "models/settingsmodel.h"
#include "util/graphindex.h"

#include <QSignalSpy>
#include <QTest>

using GuiState = GuiModel::GuiState;

void TestScopeController::init()
{
    _pGuiModel = new GuiModel(this);
    _pSettingsModel = new SettingsModel(this);
    _pGraphDataModel = new GraphDataModel(this);
    _pCommunicationStatsModel = new CommunicationStatsModel(this);
    _pNoteModel = new NoteModel(this);
    _pDataParserModel = new DataParserModel(this);

    _pController = new ScopeController(_pGuiModel, _pSettingsModel, _pGraphDataModel, _pCommunicationStatsModel,
                                       _pNoteModel, _pDataParserModel, this);
}

void TestScopeController::cleanup()
{
    delete _pController;
    _pController = nullptr;

    delete _pGuiModel;
    _pGuiModel = nullptr;
    delete _pSettingsModel;
    _pSettingsModel = nullptr;
    delete _pGraphDataModel;
    _pGraphDataModel = nullptr;
    delete _pCommunicationStatsModel;
    _pCommunicationStatsModel = nullptr;
    delete _pNoteModel;
    _pNoteModel = nullptr;
    delete _pDataParserModel;
    _pDataParserModel = nullptr;
}

void TestScopeController::startWithoutRegistersEmitsError()
{
    QSignalSpy spy(_pController, &ScopeController::errorOccurred);

    _pController->start();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(_pGuiModel->guiState(), GuiState::INIT);
}

void TestScopeController::startFromDataLoadedResetsSession()
{
    _pGraphDataModel->add();
    _pGuiModel->setGuiState(GuiState::DATA_LOADED);

    QSignalSpy spy(_pController, &ScopeController::errorOccurred);

    _pController->start();

    QCOMPARE(_pGraphDataModel->size(), 0);
    QCOMPARE(_pNoteModel->size(), 0);
    QCOMPARE(_pGuiModel->guiState(), GuiState::INIT);
    QCOMPARE(spy.count(), 1);
}

void TestScopeController::clearEmitsDataClearedAndResets()
{
    _pGraphDataModel->add();
    _pGraphDataModel->mutableDataSeries(GraphIdx(0))->add(0.0, 1.0);
    _pGuiModel->setxAxisScale(AxisMode::SCALE_MANUAL);
    _pNoteModel->add(Note(QStringLiteral("note"), QPointF(0, 0)));

    QSignalSpy spy(_pController, &ScopeController::dataCleared);

    _pController->clear();

    QVERIFY(_pGraphDataModel->dataSeries(GraphIdx(0))->isEmpty());
    QCOMPARE(_pNoteModel->size(), 0);
    QCOMPARE(_pGuiModel->xAxisScalingMode(), AxisMode::SCALE_AUTO);
    QCOMPARE(spy.count(), 1);
}

void TestScopeController::openFileSetsLastDir()
{
    // Use a .mbc path: the incompatible-adapter branch below only emits a signal, never a
    // blocking dialog, so lastDir can be asserted without depending on real file I/O.
    _pController->openFile(QStringLiteral("/tmp/nonexistent/register.mbc"));

    QCOMPARE(_pGuiModel->lastDir(), QStringLiteral("/tmp/nonexistent"));
}

void TestScopeController::openMbcIncompatibleEmitsError()
{
    QSignalSpy errorSpy(_pController, &ScopeController::errorOccurred);
    QSignalSpy mbcSpy(_pController, &ScopeController::mbcImportRequested);

    _pController->openFile(QStringLiteral("/tmp/nonexistent/register.mbc"));

    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(mbcSpy.count(), 0);
}

void TestScopeController::stopSetsStoppedState()
{
    _pController->stop();

    QCOMPARE(_pGuiModel->guiState(), GuiState::STOPPED);
}

QTEST_GUILESS_MAIN(TestScopeController)
