
#include "tst_presethandler.h"

#include "importexport/presethandler.h"

#include "../mocks/gmockutils.h"
#include "../mocks/mockpresetparser.h"
#include "gmock/gmock.h"
#include <QTest>
#include <gmock/gmock-matchers.h>

using ::testing::Return;
using namespace testing;

void TestPresetHandler::init()
{
    // IMPORTANT: This must be called before any mock object constructors
    GMockUtils::InitGoogleMock();
}

void TestPresetHandler::cleanup()
{

}

void TestPresetHandler::determinePresetEmptyList()
{
    MockPresetParser presetParser;
    auto presetHandler = new PresetHandler(&presetParser);

    EXPECT_CALL(presetParser, presetCount())
            .Times(1)
            .WillOnce(Return(0));

    QCOMPARE(presetHandler->determinePreset(""), -1);
}

void TestPresetHandler::determinePresetFail()
{
    MockPresetParser presetParser;
    auto presetHandler = new PresetHandler(&presetParser);

    EXPECT_CALL(presetParser, presetCount())
            .WillRepeatedly(Return(3));

    EXPECT_CALL(presetParser, preset(_))
            .WillRepeatedly(Invoke([](int id) -> PresetParser::Preset {
        PresetParser::Preset p;
        switch(id)
        {
        case 0: p.keyword = QString("-be"); break;
        case 1: p.keyword = QString("-us"); break;
        }
        return p;
    }));

    QCOMPARE(presetHandler->determinePreset("-it"), -1);
}

void TestPresetHandler::determinePresetSuccess()
{
    MockPresetParser presetParser;
    auto presetHandler = new PresetHandler(&presetParser);

    EXPECT_CALL(presetParser, presetCount())
            .WillRepeatedly(Return(3));

    EXPECT_CALL(presetParser, preset(_))
            .WillRepeatedly(Invoke([](int id) -> PresetParser::Preset {
        PresetParser::Preset p;
        switch(id)
        {
        case 0: p.keyword = QString("-be"); break;
        case 1: p.keyword = QString("-us"); break;
        }
        return p;
    }));

    QCOMPARE(presetHandler->determinePreset("test-us"), 1);
}

void TestPresetHandler::emptyNameList()
{
    MockPresetParser presetParser;
    auto presetHandler = new PresetHandler(&presetParser);

    EXPECT_CALL(presetParser, presetCount())
            .WillRepeatedly(Return(0));

    EXPECT_CALL(presetParser, preset(_)).Times(0);

    QCOMPARE(presetHandler->nameList().size(), 0);
}

void TestPresetHandler::nameList()
{
    MockPresetParser presetParser;
    auto presetHandler = new PresetHandler(&presetParser);

    EXPECT_CALL(presetParser, presetCount())
            .WillRepeatedly(Return(3));

    EXPECT_CALL(presetParser, preset(_))
            .WillRepeatedly(Invoke([](int id) -> PresetParser::Preset {
        PresetParser::Preset p;
        switch(id)
        {
        case 0: p.name = QString("be"); break;
        case 1: p.name = QString("us"); break;
        case 2: p.name = QString("it"); break;
        }
        return p;
    }));

    QStringList nameList = presetHandler->nameList();
    QCOMPARE(nameList.size(), 3);

    QCOMPARE(nameList[0], "be");
    QCOMPARE(nameList[1], "us");
    QCOMPARE(nameList[2], "it");
}

QTEST_GUILESS_MAIN(TestPresetHandler)
