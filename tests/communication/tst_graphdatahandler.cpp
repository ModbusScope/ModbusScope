
#include <QtTest/QtTest>

#include "tst_graphdatahandler.h"

#include "communicationhelpers.h"
#include "graphdatamodel.h"
#include "graphdatahandler.h"
#include "settingsmodel.h"

Q_DECLARE_METATYPE(Result<quint16>);

using Type = ModbusDataType::Type;

#define ADD_TEST(expr, errorPos)      QTest::newRow(expr) << QString(expr) << static_cast<int>(errorPos)

void TestGraphDataHandler::init()
{
    qRegisterMetaType<Result<quint16>>("Result<quint16>");
    qRegisterMetaType<QList<Result<quint16>> >("QList<Result<quint16>>");

    _pSettingsModel = new SettingsModel;
    _pGraphDataModel = new GraphDataModel(_pSettingsModel);

    _pSettingsModel->setInt32LittleEndian(Connection::ID_1, true);
    _pSettingsModel->setInt32LittleEndian(Connection::ID_2, true);
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

    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, Connection::ID_2, Type::UNSIGNED_16)
                                                      << ModbusRegister(40001, Connection::ID_1, Type::UNSIGNED_16)
                                                      << ModbusRegister(40002, Connection::ID_1, Type::SIGNED_32);

    GraphDataHandler dataHandler;
    QList<ModbusRegister> registerList;
    dataHandler.processActiveRegisters(_pGraphDataModel);
    dataHandler.modbusRegisterList(registerList);

    QCOMPARE(expModbusRegisters, registerList);
}

void TestGraphDataHandler::errorPosition_data()
{
    QTest::addColumn<QString>("expression");
    QTest::addColumn<int>("errorPos");

    ADD_TEST("${40001}", -1);
    ADD_TEST("10 + 5", -1);
    ADD_TEST("++", 2);
    ADD_TEST("--1", 2);
    ADD_TEST("-1-+-1", 5);
    ADD_TEST("${40001}++", 11);
    ADD_TEST("${40001@1:s16b}++", 18);
    ADD_TEST("${40001@1:s16b}  ++", 20);
    ADD_TEST("${40001@1:s16b}+${40001@1}", -1);
    ADD_TEST("${40001@1:s16b}+${40001@1}--", 29);
    ADD_TEST("${40001@1:s16b} + ${40001 @ 1 }--", 34);

    ADD_TEST("${40001}\n+1\n+${40001}", -1);

}

void TestGraphDataHandler::errorPosition()
{
    QFETCH(QString, expression);
    QFETCH(int, errorPos);

    auto exprList = QStringList() << expression;

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    GraphDataHandler dataHandler;
    QList<ModbusRegister> registerList;
    dataHandler.processActiveRegisters(_pGraphDataModel);
    dataHandler.modbusRegisterList(registerList);

    auto regResults = ResultDoubleList() << ResultDouble(1, true);
    dataHandler.handleRegisterData(regResults);

    QCOMPARE(dataHandler.expressionErrorPos(0), errorPos);
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

    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(40002, Connection::ID_1, Type::UNSIGNED_16);

    GraphDataHandler dataHandler;
    QList<ModbusRegister> registerList;
    dataHandler.processActiveRegisters(_pGraphDataModel);
    dataHandler.modbusRegisterList(registerList);

    QCOMPARE(expModbusRegisters, registerList);
}

void TestGraphDataHandler::graphData()
{
    auto exprList = QStringList() << "${40001} + ${40002}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults = ResultDoubleList() << ResultDouble(1, true)
                                      << ResultDouble(2, true);

    auto resultList = ResultDoubleList() << ResultDouble(true, 3);

    QList<QVariant> rawRegData;
    doHandleRegisterData(regResults, rawRegData);
    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}

void TestGraphDataHandler::graphDataTwice()
{
    auto exprList = QStringList() << "${40001} + ${40002}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults_1 = ResultDoubleList() << ResultDouble(1, true)
                                                << ResultDouble(2, true);

    auto regResults_2 = ResultDoubleList() << ResultDouble(3, true)
                                                << ResultDouble(4, true);

    QList<QVariant> rawRegData;
    GraphDataHandler dataHandler;

    QList<ModbusRegister> registerList;
    dataHandler.processActiveRegisters(_pGraphDataModel);
    dataHandler.modbusRegisterList(registerList);

    QSignalSpy spyDataReady(&dataHandler, &GraphDataHandler::graphDataReady);

    dataHandler.handleRegisterData(regResults_1);
    dataHandler.handleRegisterData(regResults_2);

    QCOMPARE(spyDataReady.count(), 2);

    auto resultList = ResultDoubleList() << ResultDouble(true, 3);
    rawRegData = spyDataReady.takeFirst();
    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);

    resultList = ResultDoubleList() << ResultDouble(true, 7);
    rawRegData = spyDataReady.takeFirst();
    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}

void TestGraphDataHandler::graphData_fail()
{
    auto exprList = QStringList() << "${40001} + ${40002}"
                                  << "${40001}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults = ResultDoubleList() << ResultDouble(1, true)
                                                << ResultDouble(0, false);

    auto resultList = ResultDoubleList() << ResultDouble(false, 0)
                                         << ResultDouble(true, 1);

    QList<QVariant> rawRegData;
    doHandleRegisterData(regResults, rawRegData);
    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}

void TestGraphDataHandler::doHandleRegisterData(ResultDoubleList& modbusResults, QList<QVariant>& actRawData)
{
    GraphDataHandler dataHandler;
    QList<ModbusRegister> registerList;
    dataHandler.processActiveRegisters(_pGraphDataModel);
    dataHandler.modbusRegisterList(registerList);

    QSignalSpy spyDataReady(&dataHandler, &GraphDataHandler::graphDataReady);
    dataHandler.handleRegisterData(modbusResults);

    QCOMPARE(spyDataReady.count(), 1);
    actRawData = spyDataReady.takeFirst();
}

QTEST_GUILESS_MAIN(TestGraphDataHandler)
