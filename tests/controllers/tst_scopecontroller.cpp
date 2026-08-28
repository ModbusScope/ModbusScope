#include "tst_scopecontroller.h"

#include "../models/devicelisthelpers.h"
#include "controllers/scopecontroller.h"
#include "models/communicationstatsmodel.h"
#include "models/dataparsermodel.h"
#include "models/graphdatamodel.h"
#include "models/guimodel.h"
#include "models/note.h"
#include "models/notemodel.h"
#include "models/settingsmodel.h"
#include "util/graphindex.h"

#include <QJsonArray>
#include <QJsonObject>
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

void TestScopeController::startWithUnknownDeviceEmitsError()
{
    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(GraphIdx(0), QStringLiteral("${45332@99}"));
    _pGraphDataModel->setActive(GraphIdx(0), true);

    QSignalSpy spy(_pController, &ScopeController::errorOccurred);

    _pController->start();

    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.at(0).at(0).toString().contains(QStringLiteral("99")));
    QCOMPARE(_pGuiModel->guiState(), GuiState::INIT);
    QVERIFY(!_pController->isPolling());
}

void TestScopeController::startWithDeviceTruncatedByAdapterLimitEmitsError()
{
    DeviceListHelpers::seedDevice(_pSettingsModel, 1, QStringLiteral("modbus"));
    DeviceListHelpers::seedDevice(_pSettingsModel, 2, QStringLiteral("modbus"));

    // The adapter's schema allows only one device; configForWire() drops the second.
    QJsonObject devicesSchema;
    devicesSchema["maxItems"] = 1;
    QJsonObject properties;
    properties["devices"] = devicesSchema;
    QJsonObject schema;
    schema["properties"] = properties;
    QJsonObject describeResult;
    describeResult["schema"] = schema;
    _pSettingsModel->updateAdapterFromDescribe(QStringLiteral("modbus"), describeResult);

    QJsonObject device1;
    device1["id"] = 1;
    QJsonObject device2;
    device2["id"] = 2;
    QJsonObject config;
    config["devices"] = QJsonArray({ device1, device2 });
    _pSettingsModel->setAdapterCurrentConfig(QStringLiteral("modbus"), config);

    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(GraphIdx(0), QStringLiteral("${100}"));
    _pGraphDataModel->setActive(GraphIdx(0), true);

    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(GraphIdx(1), QStringLiteral("${200@2}"));
    _pGraphDataModel->setActive(GraphIdx(1), true);

    QSignalSpy spy(_pController, &ScopeController::errorOccurred);

    _pController->start();

    QCOMPARE(spy.count(), 1);
    const QString message = spy.at(0).at(0).toString();
    QVERIFY(message.contains(QStringLiteral("Device 2")));
    QVERIFY(message.contains(QStringLiteral("device limit exceeded")));
    QCOMPARE(_pGuiModel->guiState(), GuiState::INIT);
}

void TestScopeController::startWithDeviceOwnedByUnavailableAdapterEmitsError()
{
    DeviceListHelpers::seedDevice(_pSettingsModel, 1, QStringLiteral("dummy"));

    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(GraphIdx(0), QStringLiteral("${100}"));
    _pGraphDataModel->setActive(GraphIdx(0), true);

    QSignalSpy spy(_pController, &ScopeController::errorOccurred);

    _pController->start();

    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.at(0).at(0).toString().contains(QStringLiteral("dummy")));
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
