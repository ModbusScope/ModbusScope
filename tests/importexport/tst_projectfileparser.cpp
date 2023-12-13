
#include <QtTest/QtTest>

#include "tst_projectfileparser.h"

#include "projectfileparser.h"
#include "projectfiletestdata.h"

using ProjectFileData::ProjectSettings;

void TestProjectFileParser::init()
{

}

void TestProjectFileParser::initTestCase()
{
    QLocale::setDefault(QLocale(QLocale::Dutch, QLocale::Belgium));
}

void TestProjectFileParser::cleanup()
{

}

void TestProjectFileParser::tooLowDataLevel()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    GeneralError parseError = projectParser.parseFile(ProjectFileTestData::cTooLowDataLevel, &settings);
    QVERIFY(parseError.result() == false);
}

void TestProjectFileParser::tooHighDataLevel()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    GeneralError parseError = projectParser.parseFile(ProjectFileTestData::cTooHighDataLevel, &settings);
    QVERIFY(parseError.result() == false);
}

void TestProjectFileParser::dataLevel3Expressions()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    GeneralError parseError = projectParser.parseFile(ProjectFileTestData::cDataLevel3Expressions, &settings);
    QVERIFY(parseError.result());

    QVERIFY(settings.scope.registerList[0].bActive);
    QCOMPARE(settings.scope.registerList[0].expression, QString("${40001}/2"));
    QCOMPARE(settings.scope.registerList[0].bColor, true);
    QCOMPARE(settings.scope.registerList[0].color, QColor(0xff, 0x00, 0x00));

    QVERIFY(settings.scope.registerList[1].bActive);
    QCOMPARE(settings.scope.registerList[1].expression, QString("${40002:s16b}"));
    QCOMPARE(settings.scope.registerList[1].bColor, true);
    QCOMPARE(settings.scope.registerList[1].color, QColor(0x00, 0x00, 0xff));

    QVERIFY(!settings.scope.registerList[2].bActive);
    QCOMPARE(settings.scope.registerList[2].expression, QString("${40003@2:s16b}*10"));
    QCOMPARE(settings.scope.registerList[2].bColor, false);
}

void TestProjectFileParser::connSerial()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    GeneralError parseError = projectParser.parseFile(ProjectFileTestData::cConnSerial, &settings);
    QVERIFY(parseError.result());

    /* Connection id 0 */
    QVERIFY(settings.general.connectionSettings[0].bConnectionId);
    QCOMPARE(settings.general.connectionSettings[0].connectionId, 0);

    QVERIFY(settings.general.connectionSettings[0].bConnectionState);

    QVERIFY(settings.general.connectionSettings[0].bConnectionType);
    QCOMPARE(settings.general.connectionSettings[0].connectionType, "serial");

    QVERIFY(settings.general.connectionSettings[0].bIp);
    QCOMPARE(settings.general.connectionSettings[0].ip, "127.0.0.1");

    QVERIFY(settings.general.connectionSettings[0].bPort);
    QCOMPARE(settings.general.connectionSettings[0].port, 502);

    QVERIFY(settings.general.connectionSettings[0].bPortName);
    QCOMPARE(settings.general.connectionSettings[0].portName, "/dev/ttyUSB2");

    QVERIFY(settings.general.connectionSettings[0].bBaudrate);
    QCOMPARE(settings.general.connectionSettings[0].baudrate, 9600);

    QVERIFY(settings.general.connectionSettings[0].bParity);
    QCOMPARE(settings.general.connectionSettings[0].parity, 0);

    QVERIFY(settings.general.connectionSettings[0].bStopbits);
    QCOMPARE(settings.general.connectionSettings[0].stopbits, 3);

    QVERIFY(settings.general.connectionSettings[0].bDatabits);
    QCOMPARE(settings.general.connectionSettings[0].databits, 7);

    QVERIFY(settings.general.connectionSettings[0].bSlaveId);
    QCOMPARE(settings.general.connectionSettings[0].slaveId, 3);

    QVERIFY(settings.general.connectionSettings[0].bTimeout);
    QCOMPARE(settings.general.connectionSettings[0].timeout, 1003);

    QVERIFY(settings.general.connectionSettings[0].bConsecutiveMax);
    QCOMPARE(settings.general.connectionSettings[0].consecutiveMax, 12);

    QVERIFY(settings.general.connectionSettings[0].bInt32LittleEndian);
    QVERIFY(settings.general.connectionSettings[0].bPersistentConnection);
}

void TestProjectFileParser::connMixedMulti()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    GeneralError parseError = projectParser.parseFile(ProjectFileTestData::cConnMixedMulti, &settings);
    QVERIFY(parseError.result());

    QCOMPARE(settings.general.connectionSettings.size(), 3);

    /* Connection id 0 */
    QVERIFY(settings.general.connectionSettings[0].bConnectionId);
    QCOMPARE(settings.general.connectionSettings[0].connectionId, 0);

    QVERIFY(settings.general.connectionSettings[0].bConnectionState);

    QVERIFY(settings.general.connectionSettings[0].bConnectionType);
    QCOMPARE(settings.general.connectionSettings[0].connectionType, "tcp");

    QVERIFY(settings.general.connectionSettings[0].bIp);
    QCOMPARE(settings.general.connectionSettings[0].ip, "127.0.0.2");

    QVERIFY(settings.general.connectionSettings[0].bPort);
    QCOMPARE(settings.general.connectionSettings[0].port, 5020);

    QVERIFY(settings.general.connectionSettings[0].bPortName);
    QCOMPARE(settings.general.connectionSettings[0].portName, "/dev/ttyUSB0");

    QVERIFY(settings.general.connectionSettings[0].bBaudrate);
    QCOMPARE(settings.general.connectionSettings[0].baudrate, 115200);

    QVERIFY(settings.general.connectionSettings[0].bParity);
    QCOMPARE(settings.general.connectionSettings[0].parity, 0);

    QVERIFY(settings.general.connectionSettings[0].bStopbits);
    QCOMPARE(settings.general.connectionSettings[0].stopbits, 1);

    QVERIFY(settings.general.connectionSettings[0].bDatabits);
    QCOMPARE(settings.general.connectionSettings[0].databits, 8);

    QVERIFY(settings.general.connectionSettings[0].bSlaveId);
    QCOMPARE(settings.general.connectionSettings[0].slaveId, 2);

    QVERIFY(settings.general.connectionSettings[0].bTimeout);
    QCOMPARE(settings.general.connectionSettings[0].timeout, 1002);

    QVERIFY(settings.general.connectionSettings[0].bConsecutiveMax);
    QCOMPARE(settings.general.connectionSettings[0].consecutiveMax, 12);

    QVERIFY(settings.general.connectionSettings[0].bInt32LittleEndian);
    QVERIFY(settings.general.connectionSettings[0].bPersistentConnection);


    /* Connection id 1 */
    QVERIFY(settings.general.connectionSettings[1].bConnectionId);
    QCOMPARE(settings.general.connectionSettings[1].connectionId, 1);

    QVERIFY(settings.general.connectionSettings[1].bConnectionState == false);

    QVERIFY(settings.general.connectionSettings[1].bConnectionType);
    QCOMPARE(settings.general.connectionSettings[1].connectionType, "tcp");

    QVERIFY(settings.general.connectionSettings[1].bIp);
    QCOMPARE(settings.general.connectionSettings[1].ip, "127.0.0.1");

    QVERIFY(settings.general.connectionSettings[1].bPort);
    QCOMPARE(settings.general.connectionSettings[1].port, 502);

    QVERIFY(settings.general.connectionSettings[1].bPortName);
    QCOMPARE(settings.general.connectionSettings[1].portName, "/dev/ttyUSB0");

    QVERIFY(settings.general.connectionSettings[1].bBaudrate);
    QCOMPARE(settings.general.connectionSettings[1].baudrate, 115200);

    QVERIFY(settings.general.connectionSettings[1].bParity);
    QCOMPARE(settings.general.connectionSettings[1].parity, 0);

    QVERIFY(settings.general.connectionSettings[1].bStopbits);
    QCOMPARE(settings.general.connectionSettings[1].stopbits, 1);

    QVERIFY(settings.general.connectionSettings[1].bDatabits);
    QCOMPARE(settings.general.connectionSettings[1].databits, 8);

    QVERIFY(settings.general.connectionSettings[1].bSlaveId);
    QCOMPARE(settings.general.connectionSettings[1].slaveId, 1);

    QVERIFY(settings.general.connectionSettings[1].bTimeout);
    QCOMPARE(settings.general.connectionSettings[1].timeout, 1000);

    QVERIFY(settings.general.connectionSettings[1].bConsecutiveMax);
    QCOMPARE(settings.general.connectionSettings[1].consecutiveMax, 125);

    QVERIFY(settings.general.connectionSettings[1].bInt32LittleEndian);
    QVERIFY(settings.general.connectionSettings[1].bPersistentConnection);


    /* Connection id 2 */
    QVERIFY(settings.general.connectionSettings[2].bConnectionId);
    QCOMPARE(settings.general.connectionSettings[2].connectionId, 2);

    QVERIFY(settings.general.connectionSettings[2].bConnectionState);

    QVERIFY(settings.general.connectionSettings[2].bConnectionType);
    QCOMPARE(settings.general.connectionSettings[2].connectionType, "serial");

    QVERIFY(settings.general.connectionSettings[2].bIp);
    QCOMPARE(settings.general.connectionSettings[2].ip, "127.0.0.1");

    QVERIFY(settings.general.connectionSettings[2].bPort);
    QCOMPARE(settings.general.connectionSettings[2].port, 502);

    QVERIFY(settings.general.connectionSettings[2].bPortName);
    QCOMPARE(settings.general.connectionSettings[2].portName, "/dev/ttyUSB2");

    QVERIFY(settings.general.connectionSettings[2].bBaudrate);
    QCOMPARE(settings.general.connectionSettings[2].baudrate, 9600);

    QVERIFY(settings.general.connectionSettings[2].bParity);
    QCOMPARE(settings.general.connectionSettings[2].parity, 0);

    QVERIFY(settings.general.connectionSettings[2].bStopbits);
    QCOMPARE(settings.general.connectionSettings[2].stopbits, 3);

    QVERIFY(settings.general.connectionSettings[2].bDatabits);
    QCOMPARE(settings.general.connectionSettings[2].databits, 7);

    QVERIFY(settings.general.connectionSettings[2].bSlaveId);
    QCOMPARE(settings.general.connectionSettings[2].slaveId, 3);

    QVERIFY(settings.general.connectionSettings[2].bTimeout);
    QCOMPARE(settings.general.connectionSettings[2].timeout, 1003);

    QVERIFY(settings.general.connectionSettings[2].bConsecutiveMax);
    QCOMPARE(settings.general.connectionSettings[2].consecutiveMax, 12);

    QVERIFY(settings.general.connectionSettings[2].bInt32LittleEndian);
    QVERIFY(settings.general.connectionSettings[2].bPersistentConnection);

}

void TestProjectFileParser::connEmpty()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    GeneralError parseError = projectParser.parseFile(ProjectFileTestData::cConnEmpty, &settings);
    QVERIFY(parseError.result());

    /* Connection id 0 */
    QVERIFY(settings.general.connectionSettings[0].bConnectionId == false);
    QVERIFY(settings.general.connectionSettings[0].bConnectionState);
    QVERIFY(settings.general.connectionSettings[0].bConnectionType == false);

    QVERIFY(settings.general.connectionSettings[0].bIp == false);
    QVERIFY(settings.general.connectionSettings[0].bPort == false);
    QVERIFY(settings.general.connectionSettings[0].bPortName == false);
    QVERIFY(settings.general.connectionSettings[0].bBaudrate == false);
    QVERIFY(settings.general.connectionSettings[0].bParity == false);
    QVERIFY(settings.general.connectionSettings[0].bStopbits == false);
    QVERIFY(settings.general.connectionSettings[0].bDatabits == false);

    QVERIFY(settings.general.connectionSettings[0].bSlaveId == false);
    QVERIFY(settings.general.connectionSettings[0].bTimeout == false);
    QVERIFY(settings.general.connectionSettings[0].bConsecutiveMax == false);

    QVERIFY(settings.general.connectionSettings[0].bInt32LittleEndian);
    QVERIFY(settings.general.connectionSettings[0].bPersistentConnection);
}

void TestProjectFileParser::scaleDouble()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    GeneralError parseError = projectParser.parseFile(ProjectFileTestData::cScaleDouble, &settings);
    QVERIFY(parseError.result());

    /* Connection id 0 */
    QVERIFY(settings.view.scaleSettings.bSliding);
    QCOMPARE(settings.view.scaleSettings.slidingInterval, 20);

    QVERIFY(settings.view.scaleSettings.bMinMax);
    QCOMPARE(settings.view.scaleSettings.scaleMin, 0);
    QCOMPARE(settings.view.scaleSettings.scaleMax, 25.5);
}

void TestProjectFileParser::valueAxis()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    GeneralError parseError = projectParser.parseFile(ProjectFileTestData::cValueAxis, &settings);
    QVERIFY(parseError.result());

    QCOMPARE(settings.scope.registerList[0].valueAxis, 0);
    QCOMPARE(settings.scope.registerList[1].valueAxis, 1);
    QCOMPARE(settings.scope.registerList[2].valueAxis, 0);
}


QTEST_GUILESS_MAIN(TestProjectFileParser)
