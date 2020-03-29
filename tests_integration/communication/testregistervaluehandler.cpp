
#include <QtTest/QtTest>

#include "testregistervaluehandler.h"

#include "graphdatamodel.h"
#include "settingsmodel.h"

/*
 * _1: default (read)
 * _2: multiply
 * _3: divide
 * _4: multiply and divide
 * _5: Shift
 * _6: Signed shift
 * _7: Bitmask
 * _8: Full option (to test sequence)
 * */

void TestRegisterValueHandler::init()
{
    _pSettingsModel = new SettingsModel();
    _pGraphDataModel = new GraphDataModel(_pSettingsModel);
}

void TestRegisterValueHandler::cleanup()
{
    delete _pGraphDataModel;
    delete _pSettingsModel;
}

void TestRegisterValueHandler::read16_1()
{
    addRegisterToModel();

    auto partialResultMap = createResultMap(0, false, 100, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 100);
    QCOMPARE(regHandler.successList()[0], true);

}

void TestRegisterValueHandler::read32_1()
{
    addRegisterToModel();
    _pGraphDataModel->setBit32(0, true);

     auto partialResultMap = createResultMap(0, true, 100000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 100000);
    QCOMPARE(regHandler.successList()[0], true);

}

void TestRegisterValueHandler::multiRead()
{
    addRegisterToModel();
    addRegisterToModel();
    addRegisterToModel();
    addRegisterToModel();

    _pGraphDataModel->setMultiplyFactor(0, 2);
    _pGraphDataModel->setRegisterAddress(0, 0);

    _pGraphDataModel->setDivideFactor(1, 3);
    _pGraphDataModel->setRegisterAddress(1, 3);

    _pGraphDataModel->setBit32(2, true);
    _pGraphDataModel->setShift(2, 2);
    _pGraphDataModel->setRegisterAddress(2, 5);

    _pGraphDataModel->setBitmask(3, 0x1F00);
    _pGraphDataModel->setRegisterAddress(3, 7);

    auto partialResultMap = createResultMap(0, false, 100, true);
    addToResultMap(partialResultMap, 3, false, 300, true);
    addToResultMap(partialResultMap, 5, true, 0x1F550, true);
    addToResultMap(partialResultMap, 7, true, 0x3A01, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);


    QCOMPARE(regHandler.processedValues()[0], 200);
    QCOMPARE(regHandler.successList()[0], true);

    QCOMPARE(regHandler.processedValues()[1], 100);
    QCOMPARE(regHandler.successList()[1], true);

    QCOMPARE(regHandler.processedValues()[2], 0x7D540);
    QCOMPARE(regHandler.successList()[2], true);

    QCOMPARE(regHandler.processedValues()[3], 0x1A00);
    QCOMPARE(regHandler.successList()[3], true);

}

void TestRegisterValueHandler::connectionCheck()
{
    addRegisterToModel();

    auto partialResultMap = createResultMap(0, false, 100, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();

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
    addRegisterToModel();
    addRegisterToModel();

    _pGraphDataModel->setConnectionId(1, SettingsModel::CONNECTION_ID_1);

    auto partialResultMap = createResultMap(0, false, 100, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
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

void TestRegisterValueHandler::addToResultMap(QMap<quint16, ModbusResult> &resultMap,
        quint16 addr,
        bool b32bit,
        qint64 value,
        bool result
        )
{
    resultMap.insert(addr, ModbusResult(static_cast<quint16>(value), result));

    if (b32bit)
    {
        resultMap.insert(addr + 1, ModbusResult(static_cast<quint32>(value) >> 16, result));
    }
}

void TestRegisterValueHandler::addRegisterToModel()
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
