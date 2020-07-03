
#include <QtTest/QtTest>

#include "testprojectfileparser.h"

#include "projectfileparser.h"
#include "projectfiletestdata.h"

using ProjectFileData::ProjectSettings;

void TestProjectFileParser::init()
{

}

void TestProjectFileParser::cleanup()
{

}

void TestProjectFileParser::legacyRegExpressions()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    QVERIFY(projectParser.parseFile(ProjectFileTestData::cLegacyRegExpressions, &settings));

    QCOMPARE(settings.scope.registerList[0].address, 40002);
    QCOMPARE(settings.scope.registerList[0].multiplyFactor, 3);
    QCOMPARE(settings.scope.registerList[0].divideFactor, 2);
    QCOMPARE(settings.scope.registerList[0].shift, 1);
    QCOMPARE(settings.scope.registerList[0].bitmask, 0xFF00);

    QVERIFY(!settings.scope.registerList[0].bExpression);
}

void TestProjectFileParser::newRegExpressions()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    QVERIFY(projectParser.parseFile(ProjectFileTestData::cNewRegExpressions, &settings));

    QCOMPARE(settings.scope.registerList[0].address, 40002);
    QCOMPARE(settings.scope.registerList[0].multiplyFactor, 1);
    QCOMPARE(settings.scope.registerList[0].divideFactor, 1);
    QCOMPARE(settings.scope.registerList[0].shift, 0);
    QCOMPARE(settings.scope.registerList[0].bitmask, 0xFFFFFFFF);

    QVERIFY(settings.scope.registerList[0].bExpression);
    QCOMPARE(settings.scope.registerList[0].expression, QString("(VAL+6/2)+5+5+5"));
}

void TestProjectFileParser::bothLegacyNewRegExpressions()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    QVERIFY(projectParser.parseFile(ProjectFileTestData::cBothLegacyAndNewRegExpressions, &settings));

    QCOMPARE(settings.scope.registerList[0].address, 40002);
    QCOMPARE(settings.scope.registerList[0].multiplyFactor, 3);
    QCOMPARE(settings.scope.registerList[0].divideFactor, 2);
    QCOMPARE(settings.scope.registerList[0].shift, 1);
    QCOMPARE(settings.scope.registerList[0].bitmask, 0xFF00);

    QVERIFY(settings.scope.registerList[0].bExpression);
    QCOMPARE(settings.scope.registerList[0].expression, QString("(VAL+6/2)+5+5+5"));

}

QTEST_GUILESS_MAIN(TestProjectFileParser)
