
#include "tst_presetparser.h"

#include "importexport/presetparser.h"
#include "presetfiletestdata.h"

#include <QTest>

void TestPresetFileParser::init()
{
}

void TestPresetFileParser::cleanup()
{
}

void TestPresetFileParser::singlePreset()
{
    PresetParser presetParser;

    presetParser.parsePresets(PresetFileTestData::cSinglePreset);

    QCOMPARE(presetParser.presetCount(), 1);

    QCOMPARE(presetParser.preset(0).name, "be-seconds");
    QCOMPARE(presetParser.preset(0).fieldSeparator, QChar(';'));
    QCOMPARE(presetParser.preset(0).decimalSeparator, QChar(','));
    QCOMPARE(presetParser.preset(0).thousandSeparator, QChar(' '));
    QCOMPARE(presetParser.preset(0).commentSequence, "//");
    QCOMPARE(presetParser.preset(0).column, 1);
    QCOMPARE(presetParser.preset(0).labelRow, 1);
    QCOMPARE(presetParser.preset(0).dataRow, 2);
    QCOMPARE(presetParser.preset(0).bTimeInMilliSeconds, false);
    QCOMPARE(presetParser.preset(0).keyword, "");
}

void TestPresetFileParser::invalidFile()
{
    PresetParser presetParser;

    presetParser.parsePresets(PresetFileTestData::cInvalidFile);

    QCOMPARE(presetParser.presetCount(), 0);
}

void TestPresetFileParser::singlePresetJson()
{
    PresetParser presetParser;

    presetParser.parsePresets(PresetFileTestData::cSinglePresetJson);

    QCOMPARE(presetParser.presetCount(), 1);

    QCOMPARE(presetParser.preset(0).name, "be-seconds");
    QCOMPARE(presetParser.preset(0).fieldSeparator, QChar(';'));
    QCOMPARE(presetParser.preset(0).decimalSeparator, QChar(','));
    QCOMPARE(presetParser.preset(0).thousandSeparator, QChar(' '));
    QCOMPARE(presetParser.preset(0).commentSequence, "//");
    QCOMPARE(presetParser.preset(0).column, 1);
    QCOMPARE(presetParser.preset(0).labelRow, 1);
    QCOMPARE(presetParser.preset(0).dataRow, 2);
    QCOMPARE(presetParser.preset(0).bTimeInMilliSeconds, false);
    QCOMPARE(presetParser.preset(0).keyword, "");
}

void TestPresetFileParser::invalidFileJson()
{
    PresetParser presetParser;

    presetParser.parsePresets(PresetFileTestData::cInvalidFileJson);

    QCOMPARE(presetParser.presetCount(), 0);
}

void TestPresetFileParser::skipInvalidThenLoadValid()
{
    PresetParser presetParser;

    presetParser.parsePresets(PresetFileTestData::cMixedInvalidEntries);

    QCOMPARE(presetParser.presetCount(), 1u);
    QCOMPARE(presetParser.preset(0).name, "be-seconds");
}

void TestPresetFileParser::tabSeparator()
{
    PresetParser presetParser;

    presetParser.parsePresets(PresetFileTestData::cXmlTabSeparator);

    QCOMPARE(presetParser.presetCount(), 1u);
    QCOMPARE(presetParser.preset(0).fieldSeparator, QChar('\t'));
}

void TestPresetFileParser::tabSeparatorJson()
{
    PresetParser presetParser;

    presetParser.parsePresets(PresetFileTestData::cJsonTabSeparator);

    QCOMPARE(presetParser.presetCount(), 1u);
    QCOMPARE(presetParser.preset(0).fieldSeparator, QChar('\t'));
}

void TestPresetFileParser::xmlSkipInvalidThenLoadValid()
{
    PresetParser presetParser;

    presetParser.parsePresets(PresetFileTestData::cXmlSkipInvalidLoadValid);

    QCOMPARE(presetParser.presetCount(), 1u);
    QCOMPARE(presetParser.preset(0).name, "valid");
}

void TestPresetFileParser::optionalFieldsXml()
{
    PresetParser presetParser;

    presetParser.parsePresets(PresetFileTestData::cXmlOptionalFields);

    QCOMPARE(presetParser.presetCount(), 1u);
    QCOMPARE(presetParser.preset(0).labelRow, -1);
    QCOMPARE(presetParser.preset(0).keyword, "test-keyword");
    QCOMPARE(presetParser.preset(0).bTimeInMilliSeconds, true);
}

void TestPresetFileParser::optionalFieldsJson()
{
    PresetParser presetParser;

    presetParser.parsePresets(PresetFileTestData::cJsonOptionalFields);

    QCOMPARE(presetParser.presetCount(), 1u);
    QCOMPARE(presetParser.preset(0).labelRow, -1);
    QCOMPARE(presetParser.preset(0).keyword, "test-keyword");
    QCOMPARE(presetParser.preset(0).bTimeInMilliSeconds, true);
}

void TestPresetFileParser::minimalPresetXml()
{
    PresetParser presetParser;

    presetParser.parsePresets(PresetFileTestData::cXmlMinimalPreset);

    QCOMPARE(presetParser.presetCount(), 1u);
    QCOMPARE(presetParser.preset(0).name, "minimal");
    QCOMPARE(presetParser.preset(0).commentSequence, "");
    QCOMPARE(presetParser.preset(0).column, 1u);
    QCOMPARE(presetParser.preset(0).labelRow, 1);
    QCOMPARE(presetParser.preset(0).dataRow, 1u);
    QCOMPARE(presetParser.preset(0).bTimeInMilliSeconds, true);
    QCOMPARE(presetParser.preset(0).keyword, "");
}

void TestPresetFileParser::minimalPresetJson()
{
    PresetParser presetParser;

    presetParser.parsePresets(PresetFileTestData::cJsonMinimalPreset);

    QCOMPARE(presetParser.presetCount(), 1u);
    QCOMPARE(presetParser.preset(0).name, "minimal");
    QCOMPARE(presetParser.preset(0).commentSequence, "");
    QCOMPARE(presetParser.preset(0).column, 1u);
    QCOMPARE(presetParser.preset(0).labelRow, 1);
    QCOMPARE(presetParser.preset(0).dataRow, 1u);
    QCOMPARE(presetParser.preset(0).bTimeInMilliSeconds, true);
    QCOMPARE(presetParser.preset(0).keyword, "");
}

QTEST_GUILESS_MAIN(TestPresetFileParser)
