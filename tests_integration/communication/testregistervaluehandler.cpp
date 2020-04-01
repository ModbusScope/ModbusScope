
#include <QtTest/QtTest>

#include "testregistervaluehandler.h"

#include "graphdatamodel.h"
#include "settingsmodel.h"

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

void TestRegisterValueHandler::read_16_1()
{
    addRegisterToModel();

    auto partialResultMap = createResultMap(0, false, 100, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 100);
    QCOMPARE(regHandler.successList()[0], true);

}

void TestRegisterValueHandler::read_16_2()
{
    addRegisterToModel();

    _pGraphDataModel->setMultiplyFactor(0, 2);

    auto partialResultMap = createResultMap(0, false, 60000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 120000);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_16_3()
{
    addRegisterToModel();

    _pGraphDataModel->setDivideFactor(0, 2);

    auto partialResultMap = createResultMap(0, false, 3000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 1500);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_16_4()
{
    addRegisterToModel();

    _pGraphDataModel->setDivideFactor(0, 2);
    _pGraphDataModel->setMultiplyFactor(0, 3);

    auto partialResultMap = createResultMap(0, false, 3000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 4500);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_16_5()
{
    addRegisterToModel();

    _pGraphDataModel->setShift(0, 2);
    auto partialResultMap = createResultMap(0, false, 256, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 1024);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_16_6()
{
    addRegisterToModel();

    _pGraphDataModel->setShift(0, -2);
    auto partialResultMap = createResultMap(0, false, 256, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 64);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_16_7()
{
    addRegisterToModel();

    _pGraphDataModel->setBitmask(0, 0xA5FF);
    auto partialResultMap = createResultMap(0, false, 0x0FF0, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 0x05F0);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_16_8()
{
    addRegisterToModel();

    _pGraphDataModel->setShift(0, 2);
    _pGraphDataModel->setMultiplyFactor(0, 2);
    _pGraphDataModel->setDivideFactor(0, 4);

    auto partialResultMap = createResultMap(0, false, 256, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 512);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_s16_1()
{
    addRegisterToModel();

    _pGraphDataModel->setUnsigned(0, false);

    auto partialResultMap = createResultMap(0, false, -100, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -100);
    QCOMPARE(regHandler.successList()[0], true);

}

void TestRegisterValueHandler::read_s16_2()
{
    addRegisterToModel();

    _pGraphDataModel->setMultiplyFactor(0, 2);
    _pGraphDataModel->setUnsigned(0, false);

    auto partialResultMap = createResultMap(0, false, -30000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -60000);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_s16_3()
{
    addRegisterToModel();

    _pGraphDataModel->setUnsigned(0, false);
    _pGraphDataModel->setDivideFactor(0, 2);

    auto partialResultMap = createResultMap(0, false, -3000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -1500);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_s16_4()
{
    addRegisterToModel();

    _pGraphDataModel->setUnsigned(0, false);
    _pGraphDataModel->setDivideFactor(0, 2);
    _pGraphDataModel->setMultiplyFactor(0, 3);

    auto partialResultMap = createResultMap(0, false, -3000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -4500);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_s16_5()
{
    addRegisterToModel();

    _pGraphDataModel->setUnsigned(0, false);
    _pGraphDataModel->setShift(0, 2);
    auto partialResultMap = createResultMap(0, false, -256, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -1024);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_s16_6()
{
    addRegisterToModel();

    _pGraphDataModel->setUnsigned(0, false);
    _pGraphDataModel->setShift(0, -2);
    auto partialResultMap = createResultMap(0, false, -256, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -64);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_s16_7()
{
    addRegisterToModel();

    _pGraphDataModel->setUnsigned(0, false);
    _pGraphDataModel->setBitmask(0, 0xFF00);
    auto partialResultMap = createResultMap(0, false, -1, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -1); /* Bitmask is ignored */
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_s16_8()
{
    addRegisterToModel();

    _pGraphDataModel->setUnsigned(0, false);
    _pGraphDataModel->setShift(0, 2);
    _pGraphDataModel->setMultiplyFactor(0, 2);
    _pGraphDataModel->setDivideFactor(0, 4);

    auto partialResultMap = createResultMap(0, false, -256, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -512);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_32_1()
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

void TestRegisterValueHandler::read_32_2()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setMultiplyFactor(0, 2);

    auto partialResultMap = createResultMap(0, true, 100000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 200000);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_32_3()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setDivideFactor(0, 2);

    auto partialResultMap = createResultMap(0, true, 100000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 50000);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_32_4()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setDivideFactor(0, 2);
    _pGraphDataModel->setMultiplyFactor(0, 3);

    auto partialResultMap = createResultMap(0, true, 100000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 150000);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_32_5()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setShift(0, 2);
    auto partialResultMap = createResultMap(0, true, 0x01000000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 0x04000000);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_32_6()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setShift(0, -2);
    auto partialResultMap = createResultMap(0, true, 0x08000000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 0x02000000);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_32_7()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setBitmask(0, 0x00FFFF00);
    auto partialResultMap = createResultMap(0, true, 0x05AAAA50, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 0x00AAAA00);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_32_8()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setShift(0, 2);
    _pGraphDataModel->setMultiplyFactor(0, 2);
    _pGraphDataModel->setDivideFactor(0, 4);

    auto partialResultMap = createResultMap(0, true, 11141120, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], 22282240);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_s32_1()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setUnsigned(0, false);

    auto partialResultMap = createResultMap(0, true, -100000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -100000);
    QCOMPARE(regHandler.successList()[0], true);

}

void TestRegisterValueHandler::read_s32_2()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setUnsigned(0, false);
    _pGraphDataModel->setMultiplyFactor(0, 2);

    auto partialResultMap = createResultMap(0, true, -100000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -200000);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_s32_3()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setUnsigned(0, false);
    _pGraphDataModel->setDivideFactor(0, 2);

    auto partialResultMap = createResultMap(0, true, -100000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -50000);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_s32_4()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setUnsigned(0, false);
    _pGraphDataModel->setDivideFactor(0, 2);
    _pGraphDataModel->setMultiplyFactor(0, 3);

    auto partialResultMap = createResultMap(0, true, -100000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -150000);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_s32_5()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setUnsigned(0, false);
    _pGraphDataModel->setShift(0, 2);
    auto partialResultMap = createResultMap(0, true, -100000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -400000);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_s32_6()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setUnsigned(0, false);
    _pGraphDataModel->setShift(0, -2);
    auto partialResultMap = createResultMap(0, true, -400000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -100000);
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_s32_7()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setUnsigned(0, false);
    _pGraphDataModel->setBitmask(0, 0x00FFFF00);
    auto partialResultMap = createResultMap(0, true, -100000, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();
    regHandler.processPartialResult(partialResultMap, SettingsModel::CONNECTION_ID_0);

    QCOMPARE(regHandler.processedValues()[0], -100000); /* Bitmask is ignored on signed registers */
    QCOMPARE(regHandler.successList()[0], true);
}

void TestRegisterValueHandler::read_s32_8()
{
    addRegisterToModel();

    _pGraphDataModel->setBit32(0, true);
    _pGraphDataModel->setUnsigned(0, false);
    _pGraphDataModel->setShift(0, 2);
    _pGraphDataModel->setMultiplyFactor(0, 2);
    _pGraphDataModel->setDivideFactor(0, 4);

    auto partialResultMap = createResultMap(0, true, -11141120, true);

    RegisterValueHandler regHandler(_pGraphDataModel);
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

    RegisterValueHandler regHandler(_pGraphDataModel);
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

    RegisterValueHandler regHandler(_pGraphDataModel);
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

void TestRegisterValueHandler::graphList_1()
{
    addRegisterToModel();
    addRegisterToModel();

    _pGraphDataModel->setRegisterAddress(0, 2);

    RegisterValueHandler regHandler(_pGraphDataModel);
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

    RegisterValueHandler regHandler(_pGraphDataModel);
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

    RegisterValueHandler regHandler(_pGraphDataModel);
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

    RegisterValueHandler regHandler(_pGraphDataModel);
    regHandler.startRead();

    QList<quint16> registerList;
    regHandler.activeGraphAddresList(&registerList, SettingsModel::CONNECTION_ID_0);
    QCOMPARE(registerList[0], 0);

    regHandler.activeGraphAddresList(&registerList, SettingsModel::CONNECTION_ID_1);
    QCOMPARE(registerList[0], 5);
    QCOMPARE(registerList[1], 6);
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
