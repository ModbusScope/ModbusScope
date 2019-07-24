
#include <QtTest/QtTest>

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

/*
typedef struct
{
    bool bModbusScopeDataFile;
    QChar fieldSeparator;
    QChar groupSeparator;
    QChar decimalSeparator;
    QString commentSequence;
    quint32 dataRow;
    quint32 column;
    qint32 labelRow;
    bool bTimeInMilliSeconds;
} settingsData_t;

bool updateSettings(QTextStream* pDataFileStream, settingsData_t* pSettingsData, qint32 sampleLength);
*/

void TestSettingsAuto::processOldModbusScope()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&TestData::cModbusScopeOldFormat, &settingsData));

    QCOMPARE(settingsData.bModbusScopeDataFile, true);
    QCOMPARE(settingsData.fieldSeparator, QChar(';'));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar(','));
    QCOMPARE(settingsData.commentSequence, QString("//"));
    QCOMPARE(settingsData.labelRow, static_cast<qint32>(12));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(13));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, true);

}

void TestSettingsAuto::processNewModbusScope()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&TestData::cModbusScopeNewFormat, &settingsData));

    QCOMPARE(settingsData.bModbusScopeDataFile, true);
    QCOMPARE(settingsData.fieldSeparator, QChar(','));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar('.'));
    QCOMPARE(settingsData.commentSequence, QString("//"));
    QCOMPARE(settingsData.labelRow, static_cast<qint32>(23));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(24));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, true);
}

void TestSettingsAuto::processDatasetBe()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&TestData::cDatasetBe, &settingsData));

    QCOMPARE(settingsData.bModbusScopeDataFile, false);
    QCOMPARE(settingsData.fieldSeparator, QChar(';'));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar(','));
    QCOMPARE(settingsData.commentSequence, QString(""));
    QCOMPARE(settingsData.labelRow, static_cast<qint32>(0));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(1));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, true);
}

void TestSettingsAuto::processDatasetUs()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&TestData::cDatasetUs, &settingsData));

    QCOMPARE(settingsData.bModbusScopeDataFile, false);
    QCOMPARE(settingsData.fieldSeparator, QChar(','));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar('.'));
    QCOMPARE(settingsData.commentSequence, QString(""));
    QCOMPARE(settingsData.labelRow, static_cast<qint32>(0));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(1));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, true);
}

void TestSettingsAuto::processDatasetColumn2()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&TestData::cDatasetColumn2, &settingsData));

    /* SettingsAuto isn't capable of detecting column */
    /* So we expect the wrong value of 0 */
    QCOMPARE(settingsData.column, static_cast<quint32>(0));

    /* The rest is standard */
    QCOMPARE(settingsData.bModbusScopeDataFile, false);
    QCOMPARE(settingsData.fieldSeparator, QChar(';'));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar(','));
    QCOMPARE(settingsData.commentSequence, QString(""));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(1));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, true);
}

void TestSettingsAuto::processDatasetComment()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&TestData::cDatasetComment, &settingsData));

    QCOMPARE(settingsData.bModbusScopeDataFile, false);
    QCOMPARE(settingsData.fieldSeparator, QChar(';'));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar(','));
    QCOMPARE(settingsData.commentSequence, QString("--"));
    QCOMPARE(settingsData.labelRow, static_cast<qint32>(0));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(1));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, true);
}


void TestSettingsAuto::processDatasetSigned()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&TestData::cDatasetSigned, &settingsData));

    QCOMPARE(settingsData.bModbusScopeDataFile, false);
    QCOMPARE(settingsData.fieldSeparator, QChar(';'));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar(','));
    QCOMPARE(settingsData.commentSequence, QString(""));
    QCOMPARE(settingsData.labelRow, static_cast<qint32>(0));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(1));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, true);
}


void TestSettingsAuto::processDatasetNoLabel()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&TestData::cDatasetNoLabel, &settingsData));

    /* SettingsAuto isn't capable of detecting column */
    /* So we expect the wrong value of 0 */
    QCOMPARE(settingsData.labelRow, static_cast<qint32>(0));

    /* The rest is standard */
    QCOMPARE(settingsData.bModbusScopeDataFile, false);
    QCOMPARE(settingsData.fieldSeparator, QChar(';'));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar(','));
    QCOMPARE(settingsData.commentSequence, QString(""));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(1));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, true);
}

void TestSettingsAuto::processDatasetAbsoluteDate()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&TestData::cDatasetAbsoluteDate, &settingsData));

    QCOMPARE(settingsData.bModbusScopeDataFile, false);
    QCOMPARE(settingsData.fieldSeparator, QChar(';'));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar(','));
    QCOMPARE(settingsData.commentSequence, QString(""));
    QCOMPARE(settingsData.labelRow, static_cast<qint32>(0));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(1));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, true);
}

void TestSettingsAuto::processDatasetTimeInSeconds()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&TestData::cDatasetTimeInSecond, &settingsData));

    QCOMPARE(settingsData.bModbusScopeDataFile, false);
    QCOMPARE(settingsData.fieldSeparator, QChar(';'));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar(','));
    QCOMPARE(settingsData.commentSequence, QString(""));
    QCOMPARE(settingsData.labelRow, static_cast<qint32>(0));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(1));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, false);
}

bool TestSettingsAuto::processFile(QString* pData, SettingsAuto::settingsData_t* pResultData)
{
    QTextStream dataStream(pData);

    SettingsAuto::settingsData_t settingsData;
    SettingsAuto settingsAuto;

    return settingsAuto.updateSettings(&dataStream, pResultData, _cSampleLength);

}

QTEST_GUILESS_MAIN(TestSettingsAuto)
