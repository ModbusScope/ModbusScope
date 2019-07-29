
#include <QtTest/QtTest>

#include "testdatafileparser.h"
#include "testdata.h"

#include "datafileparser.h"

void TestDataFileParser::init()
{

}

void TestDataFileParser::cleanup()
{

}

void TestDataFileParser::parseModbusScopeOldFormat()
{
    DataParserModel dataParserModel;
    QTextStream dataStream(&TestData::cModbusScopeOldFormat);
    DataFileParser::FileData fileData;
    DataFileParser dataFileParser(&dataParserModel);

    QSignalSpy spyParseError(&dataFileParser, &DataFileParser::parseErrorOccurred);

    /* Prepare parsermodel */
    dataParserModel.setFieldSeparator(QChar(';'));
    dataParserModel.setGroupSeparator(QChar(' '));
    dataParserModel.setDecimalSeparator(QChar(','));
    dataParserModel.setCommentSequence(QString("//"));
    dataParserModel.setLabelRow(static_cast<qint32>(10));
    dataParserModel.setDataRow(static_cast<quint32>(11));
    dataParserModel.setColumn(static_cast<quint32>(0));
    dataParserModel.setTimeInMilliSeconds(true);
    dataParserModel.setStmStudioCorrection(false);

    /* Process data */
    QVERIFY(dataFileParser.processDataFile(&dataStream, &fileData));

    /* Check results */
    QCOMPARE(fileData.axisLabel, QString("Time (ms)"));
    QCOMPARE(fileData.timeRow, QList<double>() << 25 << 1024 << 2025 << 3025 << 4024);
    QCOMPARE(fileData.dataLabel, QStringList() << "Register 40001" << "Register 40002" << "Register 40003");

    QList<QList<double> > dataList;
    dataList.append(QList<double>() << 6 << 8 << 10 << 0 << 2);
    dataList.append(QList<double>() << 12 << 16 << 20 << 0 << 4);
    dataList.append(QList<double>() << 18 << 24 << 30 << 0 << 6);
    QCOMPARE(fileData.dataRows, dataList);

    QVERIFY(fileData.colors.isEmpty());
    QVERIFY(fileData.notes.isEmpty());

    QCOMPARE(spyParseError.count(), 0);
}

void TestDataFileParser::parseModbusScopeNewFormat()
{
    DataParserModel dataParserModel;
    QTextStream dataStream(&TestData::cModbusScopeNewFormat);
    DataFileParser::FileData fileData;
    DataFileParser dataFileParser(&dataParserModel);

    QSignalSpy spyParseError(&dataFileParser, &DataFileParser::parseErrorOccurred);

    /* Prepare parsermodel */
    dataParserModel.setFieldSeparator(QChar(','));
    dataParserModel.setGroupSeparator(QChar(' '));
    dataParserModel.setDecimalSeparator(QChar('.'));
    dataParserModel.setCommentSequence(QString("//"));
    dataParserModel.setLabelRow(static_cast<qint32>(22));
    dataParserModel.setDataRow(static_cast<quint32>(23));
    dataParserModel.setColumn(static_cast<quint32>(0));
    dataParserModel.setTimeInMilliSeconds(true);
    dataParserModel.setStmStudioCorrection(false);

    /* Process data */
    QVERIFY(dataFileParser.processDataFile(&dataStream, &fileData));

    /* Check results */
    QCOMPARE(fileData.axisLabel, QString("Time (ms)"));
    QCOMPARE(fileData.timeRow, QList<double>() << 37 << 262 << 513 << 763 << 1012 << 1266 << 1517 << 1768 << 2017);
    QCOMPARE(fileData.dataLabel, QStringList() << "Register 40001" << "Register 40002");

    QList<QList<double> > dataList;
    dataList.append(QList<double>() << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0);
    dataList.append(QList<double>() << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0);
    QCOMPARE(fileData.dataRows, dataList);

    QCOMPARE(fileData.colors, QList<QColor>() << QColor("#000000") << QColor("#0000FF"));

    Note note;
    note.setValueData(1.667);
    note.setKeyData(800.605);
    note.setText("Test");
    QCOMPARE(fileData.notes.size(), 1);
    QCOMPARE(fileData.notes[0].text(), note.text());
    QCOMPARE(fileData.notes[0].keyData(), note.keyData());
    QCOMPARE(fileData.notes[0].valueData(), note.valueData());

    QCOMPARE(spyParseError.count(), 0);
}

void TestDataFileParser::parseDatasetBe()
{
    DataParserModel dataParserModel;
    QTextStream dataStream(&TestData::cDatasetBe);
    DataFileParser::FileData fileData;
    DataFileParser dataFileParser(&dataParserModel);

    QSignalSpy spyParseError(&dataFileParser, &DataFileParser::parseErrorOccurred);

    /* Prepare parsermodel */
    dataParserModel.setFieldSeparator(QChar(';'));
    dataParserModel.setGroupSeparator(QChar(' '));
    dataParserModel.setDecimalSeparator(QChar(','));
    dataParserModel.setCommentSequence(QString(""));
    dataParserModel.setLabelRow(static_cast<qint32>(0));
    dataParserModel.setDataRow(static_cast<quint32>(1));
    dataParserModel.setColumn(static_cast<quint32>(0));
    dataParserModel.setTimeInMilliSeconds(true);
    dataParserModel.setStmStudioCorrection(false);

    /* Process data */
    QVERIFY(dataFileParser.processDataFile(&dataStream, &fileData));

    /* Check results */
    QCOMPARE(fileData.axisLabel, QString("Time (ms)"));

    /* Simplified check */
    QCOMPARE(fileData.timeRow.first(), 48.0);
    QCOMPARE(fileData.timeRow.last(), 12059.0);
    QCOMPARE(fileData.timeRow.size(), 13);
    QCOMPARE(fileData.dataLabel, QStringList() << "Register 40001" << "Register 40002");

    QCOMPARE(fileData.dataRows.size(), 2);

    QCOMPARE(fileData.dataRows[0].first(), 0.0);
    QCOMPARE(fileData.dataRows[1].first(), 0.5);

    QCOMPARE(fileData.dataRows[0].last(), 0.0);
    QCOMPARE(fileData.dataRows[1].last(), 0.5);

    QCOMPARE(fileData.dataRows[0].size(), 13);
    QCOMPARE(fileData.dataRows[1].size(), 13);

    QVERIFY(fileData.colors.isEmpty());
    QVERIFY(fileData.notes.isEmpty());

    QCOMPARE(spyParseError.count(), 0);
}

void TestDataFileParser::parseDatasetUs()
{
    DataParserModel dataParserModel;
    QTextStream dataStream(&TestData::cDatasetUs);
    DataFileParser::FileData fileData;
    DataFileParser dataFileParser(&dataParserModel);

    QSignalSpy spyParseError(&dataFileParser, &DataFileParser::parseErrorOccurred);

    /* Prepare parsermodel */
    dataParserModel.setFieldSeparator(QChar(','));
    dataParserModel.setGroupSeparator(QChar(' '));
    dataParserModel.setDecimalSeparator(QChar('.'));
    dataParserModel.setCommentSequence(QString(""));
    dataParserModel.setLabelRow(static_cast<qint32>(0));
    dataParserModel.setDataRow(static_cast<quint32>(1));
    dataParserModel.setColumn(static_cast<quint32>(0));
    dataParserModel.setTimeInMilliSeconds(true);
    dataParserModel.setStmStudioCorrection(false);

    /* Process data */
    QVERIFY(dataFileParser.processDataFile(&dataStream, &fileData));

    /* Check results */
    QCOMPARE(fileData.axisLabel, QString("Time (ms)"));

    /* Simplified check */
    QCOMPARE(fileData.timeRow.first(), 48.0);
    QCOMPARE(fileData.timeRow.last(), 12059.0);
    QCOMPARE(fileData.timeRow.size(), 13);
    QCOMPARE(fileData.dataLabel, QStringList() << "Register 40001" << "Register 40002");

    QCOMPARE(fileData.dataRows.size(), 2);

    QCOMPARE(fileData.dataRows[0].first(), 0.0);
    QCOMPARE(fileData.dataRows[1].first(), 0.5);

    QCOMPARE(fileData.dataRows[0].last(), 0.0);
    QCOMPARE(fileData.dataRows[1].last(), 0.5);

    QCOMPARE(fileData.dataRows[0].size(), 13);
    QCOMPARE(fileData.dataRows[1].size(), 13);

    QVERIFY(fileData.colors.isEmpty());
    QVERIFY(fileData.notes.isEmpty());

    QCOMPARE(spyParseError.count(), 0);
}

void TestDataFileParser::parseDatasetColumn2()
{
    DataParserModel dataParserModel;
    QTextStream dataStream(&TestData::cDatasetColumn2);
    DataFileParser::FileData fileData;
    DataFileParser dataFileParser(&dataParserModel);

    QSignalSpy spyParseError(&dataFileParser, &DataFileParser::parseErrorOccurred);

    /* Prepare parsermodel */
    dataParserModel.setFieldSeparator(QChar(';'));
    dataParserModel.setGroupSeparator(QChar(' '));
    dataParserModel.setDecimalSeparator(QChar(','));
    dataParserModel.setCommentSequence(QString(""));
    dataParserModel.setLabelRow(static_cast<qint32>(0));
    dataParserModel.setDataRow(static_cast<quint32>(1));
    dataParserModel.setColumn(static_cast<quint32>(1));
    dataParserModel.setTimeInMilliSeconds(true);
    dataParserModel.setStmStudioCorrection(false);

    /* Process data */
    QVERIFY(dataFileParser.processDataFile(&dataStream, &fileData));

    /* Check results */
    QCOMPARE(fileData.axisLabel, QString("Time (ms)"));

    /* Simplified check */
    QCOMPARE(fileData.timeRow.first(), 48.0);
    QCOMPARE(fileData.timeRow.last(), 12059.0);
    QCOMPARE(fileData.timeRow.size(), 13);
    QCOMPARE(fileData.dataLabel, QStringList() << "Register 40001" << "Register 40002");

    QCOMPARE(fileData.dataRows.size(), 2);

    QCOMPARE(fileData.dataRows[0].first(), 0.0);
    QCOMPARE(fileData.dataRows[1].first(), 0.0);

    QCOMPARE(fileData.dataRows[0].last(), 0.0);
    QCOMPARE(fileData.dataRows[1].last(), 0.0);

    QCOMPARE(fileData.dataRows[0].size(), 13);
    QCOMPARE(fileData.dataRows[1].size(), 13);

    QVERIFY(fileData.colors.isEmpty());
    QVERIFY(fileData.notes.isEmpty());

    QCOMPARE(spyParseError.count(), 0);
}

void TestDataFileParser::parseDatasetComment()
{
    DataParserModel dataParserModel;
    QTextStream dataStream(&TestData::cDatasetComment);
    DataFileParser::FileData fileData;
    DataFileParser dataFileParser(&dataParserModel);

    QSignalSpy spyParseError(&dataFileParser, &DataFileParser::parseErrorOccurred);

    /* Prepare parsermodel */
    dataParserModel.setFieldSeparator(QChar(';'));
    dataParserModel.setGroupSeparator(QChar(' '));
    dataParserModel.setDecimalSeparator(QChar(','));
    dataParserModel.setCommentSequence(QString("--"));
    dataParserModel.setLabelRow(static_cast<qint32>(0));
    dataParserModel.setDataRow(static_cast<quint32>(1));
    dataParserModel.setColumn(static_cast<quint32>(0));
    dataParserModel.setTimeInMilliSeconds(true);
    dataParserModel.setStmStudioCorrection(false);

    /* Process data */
    QVERIFY(dataFileParser.processDataFile(&dataStream, &fileData));

    /* Check results */
    QCOMPARE(fileData.axisLabel, QString("Time (ms)"));

    /* Simplified check */
    QCOMPARE(fileData.timeRow.first(), 48.0);
    QCOMPARE(fileData.timeRow.last(), 12059.0);
    QCOMPARE(fileData.timeRow.size(), 13);
    QCOMPARE(fileData.dataLabel, QStringList() << "Register 40001" << "Register 40002");

    QCOMPARE(fileData.dataRows.size(), 2);

    QCOMPARE(fileData.dataRows[0].first(), 0.0);
    QCOMPARE(fileData.dataRows[1].first(), 0.5);

    QCOMPARE(fileData.dataRows[0].last(), 0.0);
    QCOMPARE(fileData.dataRows[1].last(), 0.5);

    QCOMPARE(fileData.dataRows[0].size(), 13);
    QCOMPARE(fileData.dataRows[1].size(), 13);

    QVERIFY(fileData.colors.isEmpty());
    QVERIFY(fileData.notes.isEmpty());

    QCOMPARE(spyParseError.count(), 0);
}

void TestDataFileParser::parseDatasetSigned()
{

}

void TestDataFileParser::parseDatasetNoLabel()
{

}

void TestDataFileParser::parseDatasetAbsoluteDate()
{

}

void TestDataFileParser::parseDatasetTimeInSecond()
{

}

QTEST_GUILESS_MAIN(TestDataFileParser)
