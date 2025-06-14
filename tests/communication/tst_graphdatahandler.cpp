
#include "tst_graphdatahandler.h"

#include "communication/graphdatahandler.h"
#include "communicationhelpers.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"

#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(Result<quint16>);

using Type = ModbusDataType::Type;
using State = ResultState::State;

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

    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(ModbusAddress(40001), Connection::ID_2, Type::UNSIGNED_16)
                                                      << ModbusRegister(ModbusAddress(40001), Connection::ID_1, Type::UNSIGNED_16)
                                                      << ModbusRegister(ModbusAddress(40002), Connection::ID_1, Type::SIGNED_32);

    GraphDataHandler dataHandler;
    QList<ModbusRegister> registerList;
    dataHandler.processActiveRegisters(_pGraphDataModel);
    dataHandler.modbusRegisterList(registerList);

    QCOMPARE(expModbusRegisters, registerList);
}

void TestGraphDataHandler::error_data()
{
    QTest::addColumn<QString>("expression");
    QTest::addColumn<int>("errorPos");
    QTest::addColumn<QMuParser::ErrorType>("errorType");

    QTest::newRow("Test 01") << QString("${40001}")                             << static_cast<int>(-1)     << QMuParser::ErrorType::NONE;
    QTest::newRow("Test 02") << QString("10 + 5")                               << static_cast<int>(-1)     << QMuParser::ErrorType::NONE;
    QTest::newRow("Test 03") << QString("++")                                   << static_cast<int>(2)      << QMuParser::ErrorType::SYNTAX;
    QTest::newRow("Test 04") << QString("--1")                                  << static_cast<int>(2)      << QMuParser::ErrorType::SYNTAX;
    QTest::newRow("Test 05") << QString("-1-+-1")                               << static_cast<int>(5)      << QMuParser::ErrorType::SYNTAX;
    QTest::newRow("Test 06") << QString("${40001}++")                           << static_cast<int>(11)     << QMuParser::ErrorType::SYNTAX;
    QTest::newRow("Test 07") << QString("${40001@1:s16b}++")                    << static_cast<int>(18)     << QMuParser::ErrorType::SYNTAX;
    QTest::newRow("Test 08") << QString("${40001@1:s16b}  ++")                  << static_cast<int>(20)     << QMuParser::ErrorType::SYNTAX;
    QTest::newRow("Test 09") << QString("${40001 @ 1 }--")                      << static_cast<int>(16)     << QMuParser::ErrorType::SYNTAX;
    QTest::newRow("Test 10") << QString("${40001}\n+1\n+${40001}")              << static_cast<int>(-1)     << QMuParser::ErrorType::NONE;
}

void TestGraphDataHandler::error()
{
    QFETCH(QString, expression);
    QFETCH(int, errorPos);
    QFETCH(QMuParser::ErrorType, errorType);

    auto exprList = QStringList() << expression;

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    GraphDataHandler dataHandler;
    QList<ModbusRegister> registerList;
    dataHandler.processActiveRegisters(_pGraphDataModel);
    dataHandler.modbusRegisterList(registerList);

    auto regResults = ResultDoubleList() << ResultDouble(1, State::SUCCESS);
    dataHandler.handleRegisterData(regResults);

    QCOMPARE(dataHandler.expressionErrorPos(0), errorPos);
    QCOMPARE(dataHandler.expressionErrorType(0), errorType);
}

void TestGraphDataHandler::sameRegisterDifferentType()
{
    QString expression = QString("${40001@1:s16b}+${40001@1}");

    auto exprList = QStringList() << expression;

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    GraphDataHandler dataHandler;
    QList<ModbusRegister> registerList;
    dataHandler.processActiveRegisters(_pGraphDataModel);
    dataHandler.modbusRegisterList(registerList);

    auto regResults = ResultDoubleList() << ResultDouble(1, State::SUCCESS) << ResultDouble(1, State::SUCCESS);
    dataHandler.handleRegisterData(regResults);

    QCOMPARE(dataHandler.expressionErrorPos(0), -1);
    QCOMPARE(dataHandler.expressionErrorType(0), QMuParser::ErrorType::NONE);
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

    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(ModbusAddress(40002), Connection::ID_1, Type::UNSIGNED_16);

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

    auto regResults = ResultDoubleList() << ResultDouble(1, State::SUCCESS)
                                      << ResultDouble(2, State::SUCCESS);

    auto resultList = ResultDoubleList() << ResultDouble(3, State::SUCCESS);

    QList<QVariant> rawRegData;
    doHandleRegisterData(regResults, rawRegData);
    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}

void TestGraphDataHandler::graphDataTwice()
{
    auto exprList = QStringList() << "${40001} + ${40002}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults_1 = ResultDoubleList() << ResultDouble(1, State::SUCCESS)
                                                << ResultDouble(2, State::SUCCESS);

    auto regResults_2 = ResultDoubleList() << ResultDouble(3, State::SUCCESS)
                                                << ResultDouble(4, State::SUCCESS);

    QList<QVariant> rawRegData;
    GraphDataHandler dataHandler;

    QList<ModbusRegister> registerList;
    dataHandler.processActiveRegisters(_pGraphDataModel);
    dataHandler.modbusRegisterList(registerList);

    QSignalSpy spyDataReady(&dataHandler, &GraphDataHandler::graphDataReady);

    dataHandler.handleRegisterData(regResults_1);
    dataHandler.handleRegisterData(regResults_2);

    QCOMPARE(spyDataReady.count(), 2);

    auto resultList = ResultDoubleList() << ResultDouble(3, State::SUCCESS);
    rawRegData = spyDataReady.takeFirst();
    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);

    resultList = ResultDoubleList() << ResultDouble(7, State::SUCCESS);
    rawRegData = spyDataReady.takeFirst();
    CommunicationHelpers::verifyReceivedDataSignal(rawRegData, resultList);
}

void TestGraphDataHandler::graphData_fail()
{
    auto exprList = QStringList() << "${40001} + ${40002}"
                                  << "${40001}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults = ResultDoubleList() << ResultDouble(1, State::SUCCESS)
                                                << ResultDouble(0, State::INVALID);

    auto resultList = ResultDoubleList() << ResultDouble(0, State::INVALID)
                                         << ResultDouble(1, State::SUCCESS);

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
