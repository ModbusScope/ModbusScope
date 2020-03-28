
#include <QtTest/QtTest>

#include "testregistervaluehandler.h"

#include "graphdatamodel.h"
#include "settingsmodel.h"

void TestRegisterValueHandler::init()
{
    _pSettingsModel = new SettingsModel();
    _pGraphDataModel = new GraphDataModel(_pSettingsModel);

    addRegister();
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

    auto partialResultMap = createResultMap(0, false, 100, true);

    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 100);

    QCOMPARE(regHandler.successList()[0], true);

}


void TestRegisterValueHandler::read32()
{
    _pGraphDataModel->setBit32(0, true);

    RegisterValueHandler regHandler(_pGraphDataModel);

    regHandler.startRead();

    auto partialResultMap = createResultMap(0, true, 100000, true);

    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 100000);

    QCOMPARE(regHandler.successList()[0], true);

}

void TestRegisterValueHandler::connectionCheck()
{
    RegisterValueHandler regHandler(_pGraphDataModel);

    regHandler.startRead();

    auto partialResultMap = createResultMap(0, false, 100, true);

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
    addRegister();
    _pGraphDataModel->setConnectionId(1, SettingsModel::CONNECTION_ID_1);

    RegisterValueHandler regHandler(_pGraphDataModel);

    regHandler.startRead();

    auto partialResultMap = createResultMap(0, false, 100, true);

    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    partialResultMap.first().setValue(101);
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_1);

    QCOMPARE(regHandler.processedValues()[0], 100);
    QCOMPARE(regHandler.successList()[0], true);
    QCOMPARE(regHandler.processedValues()[1], 101);
    QCOMPARE(regHandler.successList()[1], true);
}

QMap<quint16, ModbusResult> TestRegisterValueHandler::createResultMap(
        quint16 addr,
        bool b32bit,
        qint64 value,
        bool result
        )
{
    QMap<quint16, ModbusResult> partialResultMap;

    partialResultMap.insert(addr, ModbusResult(static_cast<quint16>(value), result));

    if (b32bit)
    {
        partialResultMap.insert(addr + 1, ModbusResult(static_cast<quint32>(value) >> 16, result));
    }

    return partialResultMap;
}

void TestRegisterValueHandler::addRegister()
{
    GraphData graphData;
    graphData.setActive(true);
    graphData.setUnsigned(true);
    graphData.setBit32(false);
    graphData.setMultiplyFactor(1);
    graphData.setDivideFactor(1);
    graphData.setRegisterAddress(0);
    graphData.setBitmask(0xFFFFFFFF);
    graphData.setShift(0);
    graphData.setConnectionId(SettingsModel::CONNECTION_ID_0);

    _pGraphDataModel->add(graphData);
}

QTEST_GUILESS_MAIN(TestRegisterValueHandler)
