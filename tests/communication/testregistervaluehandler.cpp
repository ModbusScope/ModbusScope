
#include <QtTest/QtTest>

#include "testregistervaluehandler.h"

#include "graphdatamodel.h"
#include "settingsmodel.h"

void TestRegisterValueHandler::init()
{
    _pSettingsModel = new SettingsModel();

    _pSettingsModel->setInt32LittleEndian(SettingsModel::CONNECTION_ID_0, true);
    _pSettingsModel->setInt32LittleEndian(SettingsModel::CONNECTION_ID_1, true);

    _pGraphDataModel = new GraphDataModel(_pSettingsModel);
}

void TestRegisterValueHandler::cleanup()
{
    delete _pGraphDataModel;
    delete _pSettingsModel;
}

void TestRegisterValueHandler::read_16_default()
{
    addRegisterToModel();

    auto partialResultMap = createResultMap(0, false, 100, true);

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 100);
    QCOMPARE(regHandler.successList()[0], true);

}

void TestRegisterValueHandler::read_16_operation()
{
    addRegisterToModel();

    _pGraphDataModel->setExpression(0, QStringLiteral("(VAL<<2)*2/4"));

    auto partialResultMap = createResultMap(0, false, 256, true);

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 512);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_s16_default()
{
    addRegisterToModel();

    _pGraphDataModel->setUnsigned(0, false);

    auto partialResultMap = createResultMap(0, false, -100, true);

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -100);
    QCOMPARE(regHandler.successList()[0], true);

}

void TestRegisterValueHandler::read_s16_operation()
{
    addRegisterToModel();

    _pGraphDataModel->setUnsigned(0, false);

    _pGraphDataModel->setExpression(0, QStringLiteral("(VAL<<2)*2/4"));

    auto partialResultMap = createResultMap(0, false, -256, true);

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -512);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_32_default()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);

    auto partialResultMap = createResultMap(0, true, 100000, true);

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 100000);
    QCOMPARE(regHandler.successList()[0], true);

}

void TestRegisterValueHandler::read_32_operation()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setExpression(0, QStringLiteral("(VAL<<2)*2/4"));

    auto partialResultMap = createResultMap(0, true, 11141120, true);

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 22282240);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_s32_default()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setUnsigned(0, false);

    auto partialResultMap = createResultMap(0, true, -100000, true);

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -100000);
    QCOMPARE(regHandler.successList()[0], true);

}

void TestRegisterValueHandler::read_s32_operation()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setUnsigned(0, false);
    _pGraphDataModel->setExpression(0, QStringLiteral("(VAL<<2)*2/4"));

    auto partialResultMap = createResultMap(0, true, -11141120, true);

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -22282240);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_32BitMixed_1()
{
    /* Read 32 bit register (40001) and 16 bit 40002 separately */

    addRegisterToModel();
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setRegisterAddress(0, 0);

    _pGraphDataModel->setBit32(1, false);
    _pGraphDataModel->setRegisterAddress(1, 1);

    auto partialResultMap = createResultMap(0, true, 0xAA55FF00, true);

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 0xAA55FF00);
    QCOMPARE(regHandler.successList()[0], true);

    QCOMPARE(regHandler.processedValues()[1], 0xAA55);
    QCOMPARE(regHandler.successList()[1], true);
}

void TestRegisterValueHandler::read_32BitMixed_2()
{
    /* Read 32 bit register (40001) and 40001 separately */

    addRegisterToModel();
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setRegisterAddress(0, 0);

    _pGraphDataModel->setBit32(1, false);
    _pGraphDataModel->setRegisterAddress(1, 0);

    auto partialResultMap = createResultMap(0, true, 0xAA55FF00, true);

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 0xAA55FF00);
    QCOMPARE(regHandler.successList()[0], true);

    QCOMPARE(regHandler.processedValues()[1], 0xFF00);
    QCOMPARE(regHandler.successList()[1], true);
}

void TestRegisterValueHandler::multiRead()
{
    /*
     * This test is added to test to make sure the correct options are used on the right indexes.
     */
    addRegisterToModel();
    addRegisterToModel();
    addRegisterToModel();
    addRegisterToModel();

    _pGraphDataModel->setExpression(0, QStringLiteral("VAL*2"));
    _pGraphDataModel->setRegisterAddress(0, 0);

    _pGraphDataModel->setExpression(1, QStringLiteral("VAL/3"));
    _pGraphDataModel->setRegisterAddress(1, 3);

    _pGraphDataModel->setBit32(2, true);
    _pGraphDataModel->setExpression(2, QStringLiteral("VAL<<2"));
    _pGraphDataModel->setRegisterAddress(2, 5);

    _pGraphDataModel->setExpression(3, QStringLiteral("VAL&0x1F00"));
    _pGraphDataModel->setRegisterAddress(3, 7);

    auto partialResultMap = createResultMap(0, false, 100, true);
    addToResultMap(partialResultMap, 3, false, 300, true);
    addToResultMap(partialResultMap, 5, true, 0x1F550, true);
    addToResultMap(partialResultMap, 7, true, 0x3A01, true);

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
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

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
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

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    partialResultMap.first().setValue(101);
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_1);

    QCOMPARE(regHandler.processedValues()[0], 100);
    QCOMPARE(regHandler.successList()[0], true);
    QCOMPARE(regHandler.processedValues()[1], 101);
    QCOMPARE(regHandler.successList()[1], true);
}

void TestRegisterValueHandler::graphList_1()
{
    addRegisterToModel();
    addRegisterToModel();

    _pGraphDataModel->setRegisterAddress(0, 2);

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();

    QList<quint16> registerList;

    regHandler.activeGraphAddresList(&registerList, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(registerList[0], 0);
    QCOMPARE(registerList[1], 2);
}

void TestRegisterValueHandler::graphList_2()
{
    addRegisterToModel();
    addRegisterToModel();

    _pGraphDataModel->setRegisterAddress(1, 2);
    _pGraphDataModel->setBit32(1, true);

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();

    QList<quint16> registerList;

    regHandler.activeGraphAddresList(&registerList, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(registerList[0], 0);
    QCOMPARE(registerList[1], 2);
    QCOMPARE(registerList[2], 3);
}

void TestRegisterValueHandler::graphList_3()
{
    addRegisterToModel();
    addRegisterToModel();

    _pGraphDataModel->setRegisterAddress(1, 0);
    _pGraphDataModel->setBit32(1, true);

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();

    QList<quint16> registerList;

    regHandler.activeGraphAddresList(&registerList, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(registerList[0], 0);
    QCOMPARE(registerList[1], 1);
}


void TestRegisterValueHandler::graphList_4()
{
    addRegisterToModel();
    addRegisterToModel();

    _pGraphDataModel->setConnectionId(0, SettingsModel::CONNECTION_ID_0);

    _pGraphDataModel->setConnectionId(1, SettingsModel::CONNECTION_ID_1);
    _pGraphDataModel->setRegisterAddress(1, 5);
    _pGraphDataModel->setBit32(1, true);

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();

    QList<quint16> registerList;
    regHandler.activeGraphAddresList(&registerList, SettingsModel::CONNECTION_ID_0);
    QCOMPARE(registerList[0], 0);

    regHandler.activeGraphAddresList(&registerList, SettingsModel::CONNECTION_ID_1);
    QCOMPARE(registerList[0], 5);
    QCOMPARE(registerList[1], 6);
}

void TestRegisterValueHandler::bigEndian_32_1()
{
    addRegisterToModel();

   _pSettingsModel->setInt32LittleEndian(SettingsModel::CONNECTION_ID_0, false);

    _pGraphDataModel->setBit32(0, true);

    QMap<quint16, ModbusResult> partialResultMap;
    quint32 value = 100000;

    partialResultMap.insert(0, ModbusResult(static_cast<quint32>(value) >> 16, true));
    partialResultMap.insert(0 + 1, ModbusResult(static_cast<quint16>(value), true));

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 100000);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::bigEndian_s32_2()
{
    addRegisterToModel();

   _pSettingsModel->setInt32LittleEndian(SettingsModel::CONNECTION_ID_0, false);

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setUnsigned(0, false);

    QMap<quint16, ModbusResult> partialResultMap;
    qint32 value = -100000;

    partialResultMap.insert(0, ModbusResult(static_cast<quint32>(value) >> 16, true));
    partialResultMap.insert(0 + 1, ModbusResult(static_cast<quint16>(value), true));

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -100000);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::manyInactiveRegisters()
{
    /*
     * This test is added to make sure inactive registers are handled correctly
     */

    for (uint32_t idx = 0; idx < 8; idx++)
    {
        addRegisterToModel();
        _pGraphDataModel->setActive(idx, false);
        _pGraphDataModel->setExpression(idx, QString("VAL + %1").arg(idx));
        _pGraphDataModel->setRegisterAddress(idx, idx);
    }

    _pGraphDataModel->setActive(6, true);
    _pGraphDataModel->setActive(7, true);


    auto partialResultMap = createResultMap(6, false, 100, true);
    addToResultMap(partialResultMap, 7, false, 100, true);

    RegisterValueHandler regHandler(_pGraphDataModel, _pSettingsModel);
    regHandler.prepareForData();
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 106);
    QCOMPARE(regHandler.successList()[0], true);

    QCOMPARE(regHandler.processedValues()[1], 107);
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

    addToResultMap(partialResultMap, addr, b32bit, value, result);

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
    _pGraphDataModel->add(graphData);
}

QTEST_GUILESS_MAIN(TestRegisterValueHandler)
