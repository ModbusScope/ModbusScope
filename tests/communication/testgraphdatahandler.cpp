
#include <QtTest/QtTest>

#include "testgraphdatahandler.h"

#include "communicationhelpers.h"
#include "graphdatamodel.h"
#include "graphdatahandler.h"
#include "settingsmodel.h"

Q_DECLARE_METATYPE(Result);

void TestGraphDataHandler::init()
{
    qRegisterMetaType<Result>("Result");
    qRegisterMetaType<QList<Result> >("QList<Result>");

    _pSettingsModel = new SettingsModel;
    _pGraphDataModel = new GraphDataModel(_pSettingsModel);

    _pSettingsModel->setInt32LittleEndian(SettingsModel::CONNECTION_ID_0, true);
    _pSettingsModel->setInt32LittleEndian(SettingsModel::CONNECTION_ID_1, true);
}

void TestGraphDataHandler::cleanup()
{
    delete _pGraphDataModel;
    delete _pSettingsModel;
}

void TestGraphDataHandler::registerList()
{
    auto exprList = QStringList() << "${40001@2} + ${40001}"
                                  << "${40002:s32b}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_1, false, true)
                                                      << ModbusRegister(40001, SettingsModel::CONNECTION_ID_0, false, true)
                                                      << ModbusRegister(40002, SettingsModel::CONNECTION_ID_0, true, false);

    GraphDataHandler dataHandler(_pGraphDataModel);
    QList<ModbusRegister> registerList;
    dataHandler.processActiveRegisters();
    dataHandler.modbusRegisterList(registerList);

    QCOMPARE(expModbusRegisters, registerList);
}

void TestGraphDataHandler::manyInactiveRegisters()
{
    /*
     * This test is added to make sure inactive registers are handled correctly
     */

    for (uint32_t idx = 0; idx < 8; idx++)
    {
        _pGraphDataModel->add();
        _pGraphDataModel->setExpression(idx, "${40001}");
        _pGraphDataModel->setActive(idx, false);
    }

    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(8, "${40002}");

    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(9, "${40003}");
    _pGraphDataModel->setActive(9, false);

    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(40002, SettingsModel::CONNECTION_ID_0, false, true);

    GraphDataHandler dataHandler(_pGraphDataModel);
    QList<ModbusRegister> registerList;
    dataHandler.processActiveRegisters();
    dataHandler.modbusRegisterList(registerList);

    QCOMPARE(expModbusRegisters, registerList);
}

void TestGraphDataHandler::graphData()
{
    auto exprList = QStringList() << "${40001} + ${40002}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults = QList<Result>() << Result(1, true)
                                            << Result(2, true);

    auto resultList = QList<bool>() << true;
    auto valueList = QList<double>() << 3;

    QList<QVariant> rawRegData;
    doHandleRegisterData(regResults, rawRegData);
    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList, valueList);
}

void TestGraphDataHandler::graphData_fail()
{
    auto exprList = QStringList() << "${40001} + ${40002}"
                                  << "${40001}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults = QList<Result>() << Result(1, true)
                                            << Result(0, false);

    auto resultList = QList<bool>() << false << true;
    auto valueList = QList<double>() << 0 << 1;

    QList<QVariant> rawRegData;
    doHandleRegisterData(regResults, rawRegData);
    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList, valueList);
}

void TestGraphDataHandler::doHandleRegisterData(QList<Result>& modbusResults, QList<QVariant>& actRawData)
{
    GraphDataHandler dataHandler(_pGraphDataModel);
    QList<ModbusRegister> registerList;
    dataHandler.processActiveRegisters();
    dataHandler.modbusRegisterList(registerList);

    QSignalSpy spyDataReady(&dataHandler, &GraphDataHandler::graphDataReady);
    dataHandler.handleRegisterData(modbusResults);

    QCOMPARE(spyDataReady.count(), 1);
    actRawData = spyDataReady.takeFirst();
}

QTEST_GUILESS_MAIN(TestGraphDataHandler)
