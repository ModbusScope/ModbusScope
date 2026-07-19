#include "tst_mbcfileimporter.h"

#include "MbcInterface/mbcfileimporter.h"
#include "mbctestdata.h"

#include <QTest>

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

void TestMbcFileImporter::importInvalidXml()
{
    // Malformed XML — not well-formed
    QString content = "<modbuscontrol><tab><name>Tab1</name>";
    MbcFileImporter mbcFileImporter(&content);

    // On XML parse failure the lists must be cleared
    QVERIFY(mbcFileImporter.registerList().isEmpty());
    QVERIFY(mbcFileImporter.tabList().isEmpty());
}

void TestMbcFileImporter::importWrongRootTag()
{
    // Root tag is not <modbuscontrol>
    QString content = "<?xml version=\"1.0\"?><wrongroot><tab><name>T</name><var><reg>40001</reg><text>R</text><type>uint16</type><rw>r</rw></var></tab></wrongroot>";
    MbcFileImporter mbcFileImporter(&content);

    QVERIFY(mbcFileImporter.registerList().isEmpty());
    QVERIFY(mbcFileImporter.tabList().isEmpty());
}

void TestMbcFileImporter::importMissingName()
{
    // var tag with no <text> element
    QString content = "<?xml version=\"1.0\"?><modbuscontrol><tab><name>T1</name><var><reg>40001</reg><type>uint16</type><rw>r</rw></var></tab></modbuscontrol>";
    MbcFileImporter mbcFileImporter(&content);

    // Parser reports error and returns empty lists
    QVERIFY(mbcFileImporter.registerList().isEmpty());
}

void TestMbcFileImporter::importMissingAddress()
{
    // var tag with no <reg> element
    QString content = "<?xml version=\"1.0\"?><modbuscontrol><tab><name>T1</name><var><text>Reg1</text><type>uint16</type><rw>r</rw></var></tab></modbuscontrol>";
    MbcFileImporter mbcFileImporter(&content);

    QVERIFY(mbcFileImporter.registerList().isEmpty());
}

void TestMbcFileImporter::importMissingRw()
{
    // var tag with no <rw> element
    QString content = "<?xml version=\"1.0\"?><modbuscontrol><tab><name>T1</name><var><reg>40001</reg><text>Reg1</text><type>uint16</type></var></tab></modbuscontrol>";
    MbcFileImporter mbcFileImporter(&content);

    QVERIFY(mbcFileImporter.registerList().isEmpty());
}

void TestMbcFileImporter::importWriteOnlyRegister()
{
    // A write-only register (rw="w") should be imported with readable=false
    QString content = "<?xml version=\"1.0\"?><modbuscontrol><tab><name>T1</name>"
                      "<var><reg>40001</reg><text>WriteReg</text><type>uint16</type><rw>w</rw></var>"
                      "</tab></modbuscontrol>";
    MbcFileImporter mbcFileImporter(&content);

    QList<MbcRegisterData> regList = mbcFileImporter.registerList();
    QCOMPARE(regList.size(), 1);
    QVERIFY(!regList[0].isReadable());
    QCOMPARE(regList[0].name(), QString("WriteReg"));
}

void TestMbcFileImporter::importUnknownType()
{
    // An unrecognised type is ignored: the var is skipped but other valid vars remain
    QString content = "<?xml version=\"1.0\"?><modbuscontrol><tab><name>T1</name>"
                      "<var><reg>40001</reg><text>Unknown</text><type>unknowntype</type><rw>r</rw></var>"
                      "<var><reg>40002</reg><text>Valid</text><type>uint16</type><rw>r</rw></var>"
                      "</tab></modbuscontrol>";
    MbcFileImporter mbcFileImporter(&content);

    QList<MbcRegisterData> regList = mbcFileImporter.registerList();
    QCOMPARE(regList.size(), 1);
    QCOMPARE(regList[0].name(), QString("Valid"));
    QCOMPARE(regList[0].registerAddress(), quint32(40002));
}

void TestMbcFileImporter::importStringType()
{
    // A string type is kept (not aborted) so it can be shown as an invalid register
    QString content = "<?xml version=\"1.0\"?><modbuscontrol><tab><name>T1</name>"
                      "<var><reg>40058</reg><text>File name</text><type>string50</type><rw>r</rw></var>"
                      "</tab></modbuscontrol>";
    MbcFileImporter mbcFileImporter(&content);

    QList<MbcRegisterData> regList = mbcFileImporter.registerList();
    QCOMPARE(regList.size(), 1);
    QCOMPARE(regList[0].name(), QString("File name"));
    QCOMPARE(regList[0].type(), MbcDataType::Type::STRING);
}

void TestMbcFileImporter::importStringTypeMixed()
{
    // A string var alongside a normal var: both are imported
    QString content = "<?xml version=\"1.0\"?><modbuscontrol><tab><name>T1</name>"
                      "<var><reg>40001</reg><text>Value</text><type>uint16</type><rw>r</rw></var>"
                      "<var><reg>40058</reg><text>File name</text><type>string50</type><rw>r</rw></var>"
                      "</tab></modbuscontrol>";
    MbcFileImporter mbcFileImporter(&content);

    QList<MbcRegisterData> regList = mbcFileImporter.registerList();
    QCOMPARE(regList.size(), 2);
    QCOMPARE(regList[0].type(), MbcDataType::Type::UNSIGNED_16);
    QCOMPARE(regList[1].type(), MbcDataType::Type::STRING);
}

void TestMbcFileImporter::importAutoIncrementAfterSkippedType()
{
    // A skipped unknown-type var must still advance the auto-increment address so that
    // a following <reg>*</reg> resolves instead of aborting the whole import.
    QString content = "<?xml version=\"1.0\"?><modbuscontrol><tab><name>T1</name>"
                      "<var><reg>40001</reg><text>Unknown</text><type>unknowntype</type><rw>r</rw></var>"
                      "<var><reg>*</reg><text>Next</text><type>uint16</type><rw>r</rw></var>"
                      "</tab></modbuscontrol>";
    MbcFileImporter mbcFileImporter(&content);

    QList<MbcRegisterData> regList = mbcFileImporter.registerList();
    QCOMPARE(regList.size(), 1);
    QCOMPARE(regList[0].name(), QString("Next"));
    QCOMPARE(regList[0].registerAddress(), quint32(40002));
}

void TestMbcFileImporter::importAutoIncrementAfterString()
{
    // A string50 occupies 25 registers (50 bytes), so a following <reg>*</reg> must be
    // placed after the whole string field, not one register later.
    QString content = "<?xml version=\"1.0\"?><modbuscontrol><tab><name>T1</name>"
                      "<var><reg>40100</reg><text>Str</text><type>string50</type><rw>r</rw></var>"
                      "<var><reg>*</reg><text>Next</text><type>uint16</type><rw>r</rw></var>"
                      "</tab></modbuscontrol>";
    MbcFileImporter mbcFileImporter(&content);

    QList<MbcRegisterData> regList = mbcFileImporter.registerList();
    QCOMPARE(regList.size(), 2);
    QCOMPARE(regList[0].type(), MbcDataType::Type::STRING);
    QCOMPARE(regList[1].registerAddress(), quint32(40125));
}

void TestMbcFileImporter::importMixedReadWriteRegisters()
{
    // Mix of readable and non-readable registers; all should be present
    QString content = "<?xml version=\"1.0\"?><modbuscontrol><tab><name>T1</name>"
                      "<var><reg>40001</reg><text>R1</text><type>uint16</type><rw>r</rw></var>"
                      "<var><reg>40002</reg><text>W1</text><type>uint16</type><rw>w</rw></var>"
                      "<var><reg>40003</reg><text>RW1</text><type>uint16</type><rw>rw</rw></var>"
                      "</tab></modbuscontrol>";
    MbcFileImporter mbcFileImporter(&content);

    QList<MbcRegisterData> regList = mbcFileImporter.registerList();
    QCOMPARE(regList.size(), 3);
    QVERIFY(regList[0].isReadable());
    QVERIFY(!regList[1].isReadable());
    QVERIFY(regList[2].isReadable());
}

void TestMbcFileImporter::importEmptyVarTag()
{
    // An entirely empty <var/> tag should be silently skipped
    QString content = "<?xml version=\"1.0\"?><modbuscontrol><tab><name>T1</name>"
                      "<var/>"
                      "<var><reg>40001</reg><text>Reg1</text><type>uint16</type><rw>r</rw></var>"
                      "</tab></modbuscontrol>";
    MbcFileImporter mbcFileImporter(&content);

    QList<MbcRegisterData> regList = mbcFileImporter.registerList();
    QCOMPARE(regList.size(), 1);
    QCOMPARE(regList[0].registerAddress(), quint32(40001));
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
