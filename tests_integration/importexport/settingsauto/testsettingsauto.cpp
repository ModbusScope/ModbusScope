
#include <QtTest/QtTest>

#include "settingsauto.h"

#include "testsettingsauto.h"

#include "testdata.h"

void TestSettingsAuto::init()
{

}

void TestSettingsAuto::cleanup()
{

}

void TestSettingsAuto::loadDataFullSample()
{
    QStringList list;
    QTextStream dataStream(&TestData::cModbusScopeOldFormat);

    SettingsAuto::loadDataFileSample(&dataStream, &list, 100);

    QStringList refList = TestData::cModbusScopeOldFormat.split(QRegExp("\n|\r\n|\r"));

    QCOMPARE(refList.size(), 18);
    QCOMPARE(refList, list);
}

void TestSettingsAuto::loadDataFullSampleLimited()
{
    QStringList list;
    QTextStream dataStream(&TestData::cModbusScopeOldFormat);

    const qint32 maxLine = 16;

    SettingsAuto::loadDataFileSample(&dataStream, &list, maxLine);

    QStringList refList = TestData::cModbusScopeOldFormat.split(QRegExp("\n|\r\n|\r"));

    QCOMPARE(list.size(), 16);
    for (int i = 0; i < maxLine; i++)
    {
        QCOMPARE(refList[i], list[i]);
    }
}


QTEST_GUILESS_MAIN(TestSettingsAuto)
