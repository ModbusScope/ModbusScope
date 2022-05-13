
#include <QtTest/QtTest>

#include "tst_presetparser.h"

#include "presetparser.h"
#include "presetfiletestdata.h"

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
    QCOMPARE(presetParser.preset(0).bStmStudioCorrection, false);
}

void TestPresetFileParser::invalidFile()
{
    PresetParser presetParser;

    presetParser.parsePresets(PresetFileTestData::cInvalidFile);

    QCOMPARE(presetParser.presetCount(), 0);
}



QTEST_GUILESS_MAIN(TestPresetFileParser)
