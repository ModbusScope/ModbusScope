
#include <QtTest/QtTest>
#include <QRegularExpression>

#include "tst_settingsauto.h"

#include "csvdata.h"

void TestSettingsAuto::init()
{

}

void TestSettingsAuto::cleanup()
{

}

void TestSettingsAuto::loadDataFullSample()
{
    QStringList list;
    QTextStream dataStream(&CsvData::cModbusScopeOldFormat);

    SettingsAuto::loadDataFileSample(&dataStream, list, 100);

    QStringList refList;
    prepareReference(&CsvData::cModbusScopeOldFormat, refList);

    QCOMPARE(refList.size(), 16);
    QCOMPARE(refList, list);
}

void TestSettingsAuto::loadDataFullSampleLimited()
{
    QStringList list;
    QTextStream dataStream(&CsvData::cModbusScopeOldFormat);

    const qint32 maxLine = 14;

    SettingsAuto::loadDataFileSample(&dataStream, list, maxLine);

    QStringList refList;
    prepareReference(&CsvData::cModbusScopeOldFormat, refList);

    QCOMPARE(list.size(), maxLine);
    for (int i = 0; i < maxLine; i++)
    {
        QCOMPARE(refList[i], list[i]);
    }
}

void TestSettingsAuto::processOldModbusScope()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&CsvData::cModbusScopeOldFormat, &settingsData));

    QCOMPARE(settingsData.bModbusScopeDataFile, true);
    QCOMPARE(settingsData.fieldSeparator, QChar(';'));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar(','));
    QCOMPARE(settingsData.commentSequence, QString("//"));
    QCOMPARE(settingsData.labelRow, static_cast<quint32>(10));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(11));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, true);

}

void TestSettingsAuto::processNewModbusScope()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&CsvData::cModbusScopeNewFormat, &settingsData));

    QCOMPARE(settingsData.bModbusScopeDataFile, true);
    QCOMPARE(settingsData.fieldSeparator, QChar(','));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar('.'));
    QCOMPARE(settingsData.commentSequence, QString("//"));
    QCOMPARE(settingsData.labelRow, static_cast<quint32>(22));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(23));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, true);
}

void TestSettingsAuto::processDatasetBe()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&CsvData::cDatasetBe, &settingsData));

    QCOMPARE(settingsData.bModbusScopeDataFile, false);
    QCOMPARE(settingsData.fieldSeparator, QChar(';'));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar(','));
    QCOMPARE(settingsData.commentSequence, QString(""));
    QCOMPARE(settingsData.labelRow, static_cast<quint32>(0));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(1));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, true);
}

void TestSettingsAuto::processDatasetUs()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&CsvData::cDatasetUs, &settingsData));

    QCOMPARE(settingsData.bModbusScopeDataFile, false);
    QCOMPARE(settingsData.fieldSeparator, QChar(','));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar('.'));
    QCOMPARE(settingsData.commentSequence, QString(""));
    QCOMPARE(settingsData.labelRow, static_cast<quint32>(0));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(1));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, true);
}

void TestSettingsAuto::processDatasetColumn2()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&CsvData::cDatasetColumn2, &settingsData));

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

    QVERIFY(processFile(&CsvData::cDatasetComment, &settingsData));

    QCOMPARE(settingsData.bModbusScopeDataFile, false);
    QCOMPARE(settingsData.fieldSeparator, QChar(';'));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar(','));
    QCOMPARE(settingsData.commentSequence, QString("--"));
    QCOMPARE(settingsData.labelRow, static_cast<quint32>(0));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(1));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, true);
}


void TestSettingsAuto::processDatasetSigned()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&CsvData::cDatasetSigned, &settingsData));

    QCOMPARE(settingsData.bModbusScopeDataFile, false);
    QCOMPARE(settingsData.fieldSeparator, QChar(';'));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar(','));
    QCOMPARE(settingsData.commentSequence, QString(""));
    QCOMPARE(settingsData.labelRow, static_cast<quint32>(0));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(1));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, true);
}

void TestSettingsAuto::processDatasetAbsoluteDate()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&CsvData::cDatasetAbsoluteDate, &settingsData));

    QCOMPARE(settingsData.bModbusScopeDataFile, false);
    QCOMPARE(settingsData.fieldSeparator, QChar(';'));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar(','));
    QCOMPARE(settingsData.commentSequence, QString(""));
    QCOMPARE(settingsData.labelRow, static_cast<quint32>(0));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(1));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, true);
}

void TestSettingsAuto::processDatasetTimeInSeconds()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&CsvData::cDatasetTimeInSecond, &settingsData));

    QCOMPARE(settingsData.bModbusScopeDataFile, false);
    QCOMPARE(settingsData.fieldSeparator, QChar(';'));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar(','));
    QCOMPARE(settingsData.commentSequence, QString(""));
    QCOMPARE(settingsData.labelRow, static_cast<quint32>(0));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(1));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, false);
}

void TestSettingsAuto::processDatasetExcelChanged()
{
    SettingsAuto::settingsData_t settingsData;

    QVERIFY(processFile(&CsvData::cDatasetExcelChanged, &settingsData));

    QCOMPARE(settingsData.bModbusScopeDataFile, true);
    QCOMPARE(settingsData.fieldSeparator, QChar(';'));
    QCOMPARE(settingsData.groupSeparator, QChar(' '));
    QCOMPARE(settingsData.decimalSeparator, QChar(','));
    QCOMPARE(settingsData.commentSequence, QString("//"));
    QCOMPARE(settingsData.labelRow, static_cast<quint32>(18));
    QCOMPARE(settingsData.dataRow, static_cast<quint32>(19));
    QCOMPARE(settingsData.column, static_cast<quint32>(0));
    QCOMPARE(settingsData.bTimeInMilliSeconds, true);
}


void TestSettingsAuto::prepareReference(QString* pRefData, QStringList& refList)
{
    refList = pRefData->split(QRegularExpression("\n|\r\n|\r"));

    for (int i = refList.size() - 1; i > 0; i--)
    {
        if (refList[i].trimmed().isEmpty())
        {
           refList.removeAt(i);
        }
    }
}

bool TestSettingsAuto::processFile(QString* pData, SettingsAuto::settingsData_t* pResultData)
{
    QTextStream dataStream(pData);

    SettingsAuto::settingsData_t settingsData;
    SettingsAuto settingsAuto;

    return settingsAuto.updateSettings(&dataStream, pResultData, _cSampleLength);

}

QTEST_GUILESS_MAIN(TestSettingsAuto)
