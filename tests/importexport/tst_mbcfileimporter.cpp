
#include "tst_mbcfileimporter.h"

#include "importexport/mbcfileimporter.h"
#include "mbctestdata.h"

#include <QtTest/QtTest>

void TestMbcFileImporter::init()
{

}

void TestMbcFileImporter::cleanup()
{

}

void TestMbcFileImporter::importSingleTab()
{
    MbcFileImporter mbcFileImporter(&MbcTestData::cSingleTab);

    QList <MbcRegisterData> regList = mbcFileImporter.registerList();
    QStringList tabList = mbcFileImporter.tabList();

    verifyRegList(MbcTestData::cSingleTab_RegList, regList);

    QCOMPARE(tabList, MbcTestData::cSingleTab_TabList);
}

void TestMbcFileImporter::importMultiTab()
{
    MbcFileImporter mbcFileImporter(&MbcTestData::cMultiTab);

    QList <MbcRegisterData> regList = mbcFileImporter.registerList();
    QStringList tabList = mbcFileImporter.tabList();

    verifyRegList(MbcTestData::cMultiTab_RegList, regList);

    QCOMPARE(tabList, MbcTestData::cMultiTab_TabList);
}

void TestMbcFileImporter::importRegisterOptions()
{
    MbcFileImporter mbcFileImporter(&MbcTestData::cRegisterOptions);

    QList <MbcRegisterData> regList = mbcFileImporter.registerList();
    QStringList tabList = mbcFileImporter.tabList();

    verifyRegList(MbcTestData::cRegisterOptions_RegList, regList);

    QCOMPARE(tabList, MbcTestData::cRegisterOptions_TabList);
}

void TestMbcFileImporter::importAutoIncrement()
{

    MbcFileImporter mbcFileImporter(&MbcTestData::cAutoincrement);

    QList <MbcRegisterData> regList = mbcFileImporter.registerList();
    QStringList tabList = mbcFileImporter.tabList();

    verifyRegList(MbcTestData::cAutoincrement_RegList, regList);

    QCOMPARE(tabList, MbcTestData::cAutoincrement_TabList);

}

void TestMbcFileImporter::verifyRegList(QList <MbcRegisterData> list1, QList <MbcRegisterData> list2)
{
    QVERIFY(list1.size() == list2.size());

    for (int32_t i = 0; i < list1.size(); i++)
    {
        QVERIFY(list1[i].compare(&list2[i]));
    }
}

QTEST_GUILESS_MAIN(TestMbcFileImporter)
