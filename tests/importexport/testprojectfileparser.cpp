
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

void TestProjectFileParser::connLegacySingle()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    QVERIFY(projectParser.parseFile(ProjectFileTestData::cConnLegacySingle, &settings));

    QVERIFY(settings.general.connectionSettings[0].bIp);
    QCOMPARE(settings.general.connectionSettings[0].ip, "127.0.0.2");

    QVERIFY(settings.general.connectionSettings[0].bConnectionId == false);

    QVERIFY(settings.general.connectionSettings[0].bPort);
    QCOMPARE(settings.general.connectionSettings[0].port, 5020);

    QVERIFY(settings.general.connectionSettings[0].bSlaveId);
    QCOMPARE(settings.general.connectionSettings[0].slaveId, 2);

    QVERIFY(settings.general.connectionSettings[0].bTimeout);
    QCOMPARE(settings.general.connectionSettings[0].timeout, 10000);

    QVERIFY(settings.general.connectionSettings[0].bConsecutiveMax);
    QCOMPARE(settings.general.connectionSettings[0].consecutiveMax, 200);

    QVERIFY(settings.general.connectionSettings[0].bInt32LittleEndian);
    QVERIFY(settings.general.connectionSettings[0].bPersistentConnection);
}


void TestProjectFileParser::connLegacyDual()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    QVERIFY(projectParser.parseFile(ProjectFileTestData::cConnLegacyDual, &settings));

    /* Connection id 0 */
    QVERIFY(settings.general.connectionSettings[0].bIp);
    QCOMPARE(settings.general.connectionSettings[0].ip, "127.0.0.2");

    QVERIFY(settings.general.connectionSettings[0].bConnectionId);
    QCOMPARE(settings.general.connectionSettings[0].connectionId, 0);

    QVERIFY(settings.general.connectionSettings[0].bPort);
    QCOMPARE(settings.general.connectionSettings[0].port, 5020);

    QVERIFY(settings.general.connectionSettings[0].bSlaveId);
    QCOMPARE(settings.general.connectionSettings[0].slaveId, 2);

    QVERIFY(settings.general.connectionSettings[0].bTimeout);
    QCOMPARE(settings.general.connectionSettings[0].timeout, 1001);

    QVERIFY(settings.general.connectionSettings[0].bConsecutiveMax);
    QCOMPARE(settings.general.connectionSettings[0].consecutiveMax, 250);

    QVERIFY(settings.general.connectionSettings[0].bInt32LittleEndian);
    QVERIFY(settings.general.connectionSettings[0].bPersistentConnection);

    /* Connection id 1 */
    QVERIFY(settings.general.connectionSettings[1].bIp);
    QCOMPARE(settings.general.connectionSettings[1].ip, "127.0.0.3");

    QVERIFY(settings.general.connectionSettings[1].bConnectionId);
    QCOMPARE(settings.general.connectionSettings[1].connectionId, 1);

    QVERIFY(settings.general.connectionSettings[1].bPort);
    QCOMPARE(settings.general.connectionSettings[1].port, 5021);

    QVERIFY(settings.general.connectionSettings[1].bSlaveId);
    QCOMPARE(settings.general.connectionSettings[1].slaveId, 3);

    QVERIFY(settings.general.connectionSettings[1].bTimeout);
    QCOMPARE(settings.general.connectionSettings[1].timeout, 1002);

    QVERIFY(settings.general.connectionSettings[1].bConsecutiveMax);
    QCOMPARE(settings.general.connectionSettings[1].consecutiveMax, 251);

    QVERIFY(settings.general.connectionSettings[1].bInt32LittleEndian);
    QVERIFY(settings.general.connectionSettings[1].bPersistentConnection);

}

QTEST_GUILESS_MAIN(TestProjectFileParser)
