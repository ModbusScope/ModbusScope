
#include <QtTest/QtTest>

#include "testregistervaluehandler.h"

#include "graphdatamodel.h"
#include "settingsmodel.h"

void TestRegisterValueHandler::init()
{
    _pSettingsModel = new SettingsModel();
    _pGraphDataModel = new GraphDataModel(_pSettingsModel);

    GraphData graphData;

    graphData.setActive(true);
    graphData.setUnsigned(true);
    graphData.setBit32(false);
    graphData.setMultiplyFactor(1);
    graphData.setDivideFactor(1);
    graphData.setRegisterAddress(0);
    graphData.setBitmask(0xFFFF);
    graphData.setShift(0);
    graphData.setConnectionId(SettingsModel::CONNECTION_ID_0);

    _pGraphDataModel->add(graphData);
}

void TestRegisterValueHandler::cleanup()
{
    delete _pGraphDataModel;
    delete _pSettingsModel;
}

void TestRegisterValueHandler::read16()
{
    RegisterValueHandler regHandler(_pGraphDataModel);

    regHandler.startRead();

    QMap<quint16, ModbusResult> partialResultMap;
    partialResultMap.insert(0, ModbusResult(100, true));

    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 100);

    QCOMPARE(regHandler.successList()[0], true);

}


void TestRegisterValueHandler::connectionCheck()
{
    RegisterValueHandler regHandler(_pGraphDataModel);

    regHandler.startRead();

    QMap<quint16, ModbusResult> partialResultMap;
    partialResultMap.insert(0, ModbusResult(100, true));

    /* Wrong connection */
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_1);
    QCOMPARE(regHandler.processedValues()[0], 0);
    QCOMPARE(regHandler.successList()[0], false);

    /* Correct connection */
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);
    QCOMPARE(regHandler.processedValues()[0], 100);
    QCOMPARE(regHandler.successList()[0], true);

}

void TestRegisterValueHandler::twoConnectionsCheck()
{
    GraphData graphData;

    graphData.setActive(true);
    graphData.setUnsigned(true);
    graphData.setBit32(false);
    graphData.setMultiplyFactor(1);
    graphData.setDivideFactor(1);
    graphData.setRegisterAddress(0);
    graphData.setBitmask(0xFFFF);
    graphData.setShift(0);
    graphData.setConnectionId(SettingsModel::CONNECTION_ID_1); /* Second connection */

    _pGraphDataModel->add(graphData);

    RegisterValueHandler regHandler(_pGraphDataModel);

    regHandler.startRead();

    QMap<quint16, ModbusResult> partialResultMap;
    partialResultMap.insert(0, ModbusResult(100, true));

    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    partialResultMap.first().setValue(101);
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_1);

    QCOMPARE(regHandler.processedValues()[0], 100);
    QCOMPARE(regHandler.successList()[0], true);
    QCOMPARE(regHandler.processedValues()[1], 101);
    QCOMPARE(regHandler.successList()[1], true);
}

QTEST_GUILESS_MAIN(TestRegisterValueHandler)
