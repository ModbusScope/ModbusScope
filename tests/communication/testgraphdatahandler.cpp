
#include <QtTest/QtTest>

#include "testgraphdatahandler.h"

#include "graphdatamodel.h"
#include "graphdatahandler.h"
#include "settingsmodel.h"

Q_DECLARE_METATYPE(ModbusResult);

void TestGraphDataHandler::init()
{
    qRegisterMetaType<ModbusResult>("ModbusResult");
    qRegisterMetaType<QList<ModbusResult> >("QList<ModbusResult>");

    _pSettingsModel = new SettingsModel;
    _pGraphDataModel = new GraphDataModel(_pSettingsModel);

    _pSettingsModel->setInt32LittleEndian(SettingsModel::CONNECTION_ID_0, true);
    _pSettingsModel->setInt32LittleEndian(SettingsModel::CONNECTION_ID_1, true);
}

void TestGraphDataHandler::cleanup()
{
    delete _pSettingsModel;
}

void TestGraphDataHandler::registerList()
{
    auto exprList = QStringList() << "${40001@2} + ${40001}"
                                  << "${40002:s32b}";

    addExpressionsToModel(_pGraphDataModel, exprList);

    auto expModbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_1, false, true)
                                                      << ModbusRegister(40001, SettingsModel::CONNECTION_ID_0, false, true)
                                                      << ModbusRegister(40002, SettingsModel::CONNECTION_ID_0, true, false);

    GraphDataHandler dataHandler(_pGraphDataModel);
    QList<ModbusRegister> registerList;

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

    dataHandler.modbusRegisterList(registerList);

    QCOMPARE(expModbusRegisters, registerList);
}

void TestGraphDataHandler::graphData()
{
    auto exprList = QStringList() << "${40001} + ${40002}";

    addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults = QList<ModbusResult>() << ModbusResult(1, true)
                                            << ModbusResult(2, true);

    auto resultList = QList<bool>() << true;
    auto valueList = QList<double>() << 3;

    QList<QVariant> rawRegData;
    doHandleRegisterData(regResults, rawRegData);
    verifyReceivedDataSignal(rawRegData, resultList, valueList);
}

void TestGraphDataHandler::graphData_fail()
{
    auto exprList = QStringList() << "${40001} + ${40002}"
                                  << "${40001}";

    addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults = QList<ModbusResult>() << ModbusResult(1, true)
                                            << ModbusResult(0, false);

    auto resultList = QList<bool>() << false << true;
    auto valueList = QList<double>() << 0 << 1;

    QList<QVariant> rawRegData;
    doHandleRegisterData(regResults, rawRegData);
    verifyReceivedDataSignal(rawRegData, resultList, valueList);
}

void TestGraphDataHandler::doHandleRegisterData(QList<ModbusResult>& modbusResults, QList<QVariant>& actRawData)
{
    GraphDataHandler dataHandler(_pGraphDataModel);
    QList<ModbusRegister> registerList;
    dataHandler.modbusRegisterList(registerList);

    QSignalSpy spyDataReady(&dataHandler, &GraphDataHandler::graphDataReady);
    dataHandler.handleRegisterData(modbusResults);

    QCOMPARE(spyDataReady.count(), 1);
    actRawData = spyDataReady.takeFirst();
}

void TestGraphDataHandler::addExpressionsToModel(GraphDataModel* pModel, QStringList& exprList)
{
    for(const QString &expr: qAsConst(exprList))
    {
        pModel->add();
        pModel->setExpression(pModel->size() - 1, expr);
    }
}

void TestGraphDataHandler::verifyReceivedDataSignal(QList<QVariant> arguments, QList<bool> expResultList, QList<double> expValueList)
{
    /* Verify success */
    QVERIFY((arguments[0].canConvert<QList<bool> >()));
    QList<bool> resultList = arguments[0].value<QList<bool> >();
    QCOMPARE(resultList, expResultList);

    /* Verify values */
    QVERIFY((arguments[1].canConvert<QList<double> >()));
    QList<double> valueList = arguments[1].value<QList<double> >();
    QCOMPARE(valueList, expValueList);
}

QTEST_GUILESS_MAIN(TestGraphDataHandler)
