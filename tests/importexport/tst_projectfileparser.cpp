
#include "tst_projectfileparser.h"

#include "importexport/projectfileparser.h"
#include "projectfiletestdata.h"

#include <QTest>

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

void TestProjectFileParser::dataLevel4Expressions()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    GeneralError parseError = projectParser.parseFile(ProjectFileTestData::cDataLevel4Expressions, &settings);
    QVERIFY(parseError.result());

    QVERIFY(settings.scope.registerList[0].bActive);
    QCOMPARE(settings.scope.registerList[0].expression, QString("${40001}/2"));
    QCOMPARE(settings.scope.registerList[0].bColor, true);
    QCOMPARE(settings.scope.registerList[0].color, QColor(0xff, 0x00, 0x00));

    QVERIFY(settings.scope.registerList[1].bActive);
    QCOMPARE(settings.scope.registerList[1].expression, QString("${h10000:s16b}"));
    QCOMPARE(settings.scope.registerList[1].bColor, true);
    QCOMPARE(settings.scope.registerList[1].color, QColor(0x00, 0x00, 0xff));

    QVERIFY(!settings.scope.registerList[2].bActive);
    QCOMPARE(settings.scope.registerList[2].expression, QString("${i10000@2}*10"));
    QCOMPARE(settings.scope.registerList[2].bColor, false);
}

void TestProjectFileParser::dataLevel4Connections()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    GeneralError parseError = projectParser.parseFile(ProjectFileTestData::cDataLevel4Connection, &settings);
    QVERIFY(parseError.result());

    QCOMPARE(settings.general.connectionSettings.size(), 3);

    /* Connection id 0 */
    QVERIFY(settings.general.connectionSettings[0].bConnectionId);
    QCOMPARE(settings.general.connectionSettings[0].connectionId, 0);

    QVERIFY(settings.general.connectionSettings[0].bConnectionState);

    QVERIFY(settings.general.connectionSettings[0].bConnectionType);
    QCOMPARE(settings.general.connectionSettings[0].connectionType, "tcp");

    QVERIFY(settings.general.connectionSettings[0].bIp);
    QCOMPARE(settings.general.connectionSettings[0].ip, "127.0.0.1");

    QVERIFY(settings.general.connectionSettings[0].bPort);
    QCOMPARE(settings.general.connectionSettings[0].port, 502);

    QVERIFY(settings.general.connectionSettings[0].bPortName);
    QCOMPARE(settings.general.connectionSettings[0].portName, "COM1");

    QVERIFY(settings.general.connectionSettings[0].bBaudrate);
    QCOMPARE(settings.general.connectionSettings[0].baudrate, 115200);

    QVERIFY(settings.general.connectionSettings[0].bParity);
    QCOMPARE(settings.general.connectionSettings[0].parity, 0);

    QVERIFY(settings.general.connectionSettings[0].bStopbits);
    QCOMPARE(settings.general.connectionSettings[0].stopbits, 1);

    QVERIFY(settings.general.connectionSettings[0].bDatabits);
    QCOMPARE(settings.general.connectionSettings[0].databits, 8);

    QVERIFY(settings.general.connectionSettings[0].bTimeout);
    QCOMPARE(settings.general.connectionSettings[0].timeout, 1002);

    QVERIFY(settings.general.connectionSettings[0].bPersistentConnection);

    /* Connection id 1 */
    QVERIFY(settings.general.connectionSettings[1].bConnectionId);
    QCOMPARE(settings.general.connectionSettings[1].connectionId, 1);

    QVERIFY(settings.general.connectionSettings[1].bConnectionState);

    QVERIFY(settings.general.connectionSettings[1].bConnectionType);
    QCOMPARE(settings.general.connectionSettings[1].connectionType, "serial");

    QVERIFY(settings.general.connectionSettings[1].bIp);
    QCOMPARE(settings.general.connectionSettings[1].ip, "127.0.0.1");

    QVERIFY(settings.general.connectionSettings[1].bPort);
    QCOMPARE(settings.general.connectionSettings[1].port, 502);

    QVERIFY(settings.general.connectionSettings[1].bPortName);
    QCOMPARE(settings.general.connectionSettings[1].portName, "COM10");

    QVERIFY(settings.general.connectionSettings[1].bBaudrate);
    QCOMPARE(settings.general.connectionSettings[1].baudrate, 57600);

    QVERIFY(settings.general.connectionSettings[1].bParity);
    QCOMPARE(settings.general.connectionSettings[1].parity, 2);

    QVERIFY(settings.general.connectionSettings[1].bStopbits);
    QCOMPARE(settings.general.connectionSettings[1].stopbits, 2);

    QVERIFY(settings.general.connectionSettings[1].bDatabits);
    QCOMPARE(settings.general.connectionSettings[1].databits, 7);

    QVERIFY(settings.general.connectionSettings[1].bTimeout);
    QCOMPARE(settings.general.connectionSettings[1].timeout, 1003);

    QVERIFY(settings.general.connectionSettings[1].bPersistentConnection);

    /* Connection id 2 */
    QVERIFY(settings.general.connectionSettings[2].bConnectionId);
    QCOMPARE(settings.general.connectionSettings[2].connectionId, 2);

    QVERIFY(settings.general.connectionSettings[2].bConnectionState == false);

    QVERIFY(settings.general.connectionSettings[2].bConnectionType);
    QCOMPARE(settings.general.connectionSettings[2].connectionType, "tcp");

    QVERIFY(settings.general.connectionSettings[2].bIp);
    QCOMPARE(settings.general.connectionSettings[2].ip, "127.0.0.1");

    QVERIFY(settings.general.connectionSettings[2].bPort);
    QCOMPARE(settings.general.connectionSettings[2].port, 502);

    QVERIFY(settings.general.connectionSettings[2].bPortName);
    QCOMPARE(settings.general.connectionSettings[2].portName, "COM1");

    QVERIFY(settings.general.connectionSettings[2].bBaudrate);
    QCOMPARE(settings.general.connectionSettings[2].baudrate, 115200);

    QVERIFY(settings.general.connectionSettings[2].bParity);
    QCOMPARE(settings.general.connectionSettings[2].parity, 0);

    QVERIFY(settings.general.connectionSettings[2].bStopbits);
    QCOMPARE(settings.general.connectionSettings[2].stopbits, 1);

    QVERIFY(settings.general.connectionSettings[2].bDatabits);
    QCOMPARE(settings.general.connectionSettings[2].databits, 8);

    QVERIFY(settings.general.connectionSettings[2].bTimeout);
    QCOMPARE(settings.general.connectionSettings[2].timeout, 1000);

    QVERIFY(settings.general.connectionSettings[2].bPersistentConnection);

    /* Devices */
    QVERIFY(settings.general.deviceSettings.size() == 2);
    QVERIFY(settings.general.deviceSettings[0].bDeviceId == false);
    QVERIFY(settings.general.deviceSettings[0].bName == false);
    QVERIFY(settings.general.deviceSettings[0].bConnectionId);
    QCOMPARE(settings.general.deviceSettings[0].connectionId, 0);
    QVERIFY(settings.general.deviceSettings[0].bSlaveId);
    QCOMPARE(settings.general.deviceSettings[0].slaveId, 2);
    QVERIFY(settings.general.deviceSettings[0].bConsecutiveMax);
    QCOMPARE(settings.general.deviceSettings[0].consecutiveMax, 122);
    QCOMPARE(settings.general.deviceSettings[0].bInt32LittleEndian, true);

    QVERIFY(settings.general.deviceSettings[1].bDeviceId == false);
    QVERIFY(settings.general.deviceSettings[1].bName == false);
    QVERIFY(settings.general.deviceSettings[1].bConnectionId);
    QCOMPARE(settings.general.deviceSettings[1].connectionId, 1);
    QVERIFY(settings.general.deviceSettings[1].bSlaveId);
    QCOMPARE(settings.general.deviceSettings[1].slaveId, 3);
    QVERIFY(settings.general.deviceSettings[1].bConsecutiveMax);
    QCOMPARE(settings.general.deviceSettings[1].consecutiveMax, 123);
    QCOMPARE(settings.general.deviceSettings[1].bInt32LittleEndian, false);
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

    QVERIFY(settings.general.connectionSettings[0].bTimeout);
    QCOMPARE(settings.general.connectionSettings[0].timeout, 1003);

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
    QCOMPARE(settings.general.connectionSettings[0].connectionType, "serial");

    QVERIFY(settings.general.connectionSettings[0].bIp);
    QCOMPARE(settings.general.connectionSettings[0].ip, "127.0.0.1");

    QVERIFY(settings.general.connectionSettings[0].bPort);
    QCOMPARE(settings.general.connectionSettings[0].port, 502);

    QVERIFY(settings.general.connectionSettings[0].bPortName);
    QCOMPARE(settings.general.connectionSettings[0].portName, "COM10");

    QVERIFY(settings.general.connectionSettings[0].bBaudrate);
    QCOMPARE(settings.general.connectionSettings[0].baudrate, 38400);

    QVERIFY(settings.general.connectionSettings[0].bParity);
    QCOMPARE(settings.general.connectionSettings[0].parity, 0);

    QVERIFY(settings.general.connectionSettings[0].bStopbits);
    QCOMPARE(settings.general.connectionSettings[0].stopbits, 1);

    QVERIFY(settings.general.connectionSettings[0].bDatabits);
    QCOMPARE(settings.general.connectionSettings[0].databits, 8);

    QVERIFY(settings.general.connectionSettings[0].bTimeout);
    QCOMPARE(settings.general.connectionSettings[0].timeout, 500);

    QVERIFY(settings.general.connectionSettings[0].bPersistentConnection);


    /* Connection id 1 */
    QVERIFY(settings.general.connectionSettings[1].bConnectionId);
    QCOMPARE(settings.general.connectionSettings[1].connectionId, 1);

    QVERIFY(settings.general.connectionSettings[1].bConnectionState);

    QVERIFY(settings.general.connectionSettings[1].bConnectionType);
    QCOMPARE(settings.general.connectionSettings[1].connectionType, "tcp");

    QVERIFY(settings.general.connectionSettings[1].bIp);
    QCOMPARE(settings.general.connectionSettings[1].ip, "127.0.0.1");

    QVERIFY(settings.general.connectionSettings[1].bPort);
    QCOMPARE(settings.general.connectionSettings[1].port, 502);

    QVERIFY(settings.general.connectionSettings[1].bPortName);
    QCOMPARE(settings.general.connectionSettings[1].portName, "COM1");

    QVERIFY(settings.general.connectionSettings[1].bBaudrate);
    QCOMPARE(settings.general.connectionSettings[1].baudrate, 115200);

    QVERIFY(settings.general.connectionSettings[1].bParity);
    QCOMPARE(settings.general.connectionSettings[1].parity, 0);

    QVERIFY(settings.general.connectionSettings[1].bStopbits);
    QCOMPARE(settings.general.connectionSettings[1].stopbits, 1);

    QVERIFY(settings.general.connectionSettings[1].bDatabits);
    QCOMPARE(settings.general.connectionSettings[1].databits, 8);

    QVERIFY(settings.general.connectionSettings[1].bTimeout);
    QCOMPARE(settings.general.connectionSettings[1].timeout, 2000);

    QVERIFY(settings.general.connectionSettings[1].bPersistentConnection);

    /* Connection id 2 */
    QVERIFY(settings.general.connectionSettings[2].bConnectionId);
    QCOMPARE(settings.general.connectionSettings[2].connectionId, 2);

    QVERIFY(settings.general.connectionSettings[2].bConnectionState == false);

    QVERIFY(settings.general.connectionSettings[2].bConnectionType);
    QCOMPARE(settings.general.connectionSettings[2].connectionType, "tcp");

    QVERIFY(settings.general.connectionSettings[2].bIp);
    QCOMPARE(settings.general.connectionSettings[2].ip, "127.0.0.1");

    QVERIFY(settings.general.connectionSettings[2].bPort);
    QCOMPARE(settings.general.connectionSettings[2].port, 502);

    QVERIFY(settings.general.connectionSettings[2].bPortName);
    QCOMPARE(settings.general.connectionSettings[2].portName, "COM1");

    QVERIFY(settings.general.connectionSettings[2].bBaudrate);
    QCOMPARE(settings.general.connectionSettings[2].baudrate, 115200);

    QVERIFY(settings.general.connectionSettings[2].bParity);
    QCOMPARE(settings.general.connectionSettings[2].parity, 0);

    QVERIFY(settings.general.connectionSettings[2].bStopbits);
    QCOMPARE(settings.general.connectionSettings[2].stopbits, 1);

    QVERIFY(settings.general.connectionSettings[2].bDatabits);
    QCOMPARE(settings.general.connectionSettings[2].databits, 8);

    QVERIFY(settings.general.connectionSettings[2].bTimeout);
    QCOMPARE(settings.general.connectionSettings[2].timeout, 1000);

    QVERIFY(settings.general.connectionSettings[2].bPersistentConnection);

    /* Devices */
    QVERIFY(settings.general.deviceSettings.size() == 3);
    QVERIFY(settings.general.deviceSettings[0].bDeviceId);
    QCOMPARE(settings.general.deviceSettings[0].deviceId, 1);
    QVERIFY(settings.general.deviceSettings[0].bName);
    QCOMPARE(settings.general.deviceSettings[0].name, QString("Device 1 (serial 1)"));
    QVERIFY(settings.general.deviceSettings[0].bConnectionId);
    QCOMPARE(settings.general.deviceSettings[0].connectionId, 0);
    QVERIFY(settings.general.deviceSettings[0].bSlaveId);
    QCOMPARE(settings.general.deviceSettings[0].slaveId, 1);
    QVERIFY(settings.general.deviceSettings[0].bConsecutiveMax);
    QCOMPARE(settings.general.deviceSettings[0].consecutiveMax, 125);
    QCOMPARE(settings.general.deviceSettings[0].bInt32LittleEndian, true);

    QVERIFY(settings.general.deviceSettings[1].bDeviceId);
    QCOMPARE(settings.general.deviceSettings[1].deviceId, 2);
    QVERIFY(settings.general.deviceSettings[1].bName);
    QCOMPARE(settings.general.deviceSettings[1].name, QString("Device 2 (serial 2)"));
    QVERIFY(settings.general.deviceSettings[1].bConnectionId);
    QCOMPARE(settings.general.deviceSettings[1].connectionId, 0);
    QVERIFY(settings.general.deviceSettings[1].bSlaveId);
    QCOMPARE(settings.general.deviceSettings[1].slaveId, 2);
    QVERIFY(settings.general.deviceSettings[1].bConsecutiveMax);
    QCOMPARE(settings.general.deviceSettings[1].consecutiveMax, 125);
    QCOMPARE(settings.general.deviceSettings[1].bInt32LittleEndian, true);

    QVERIFY(settings.general.deviceSettings[2].bDeviceId);
    QCOMPARE(settings.general.deviceSettings[2].deviceId, 3);
    QVERIFY(settings.general.deviceSettings[2].bName);
    QCOMPARE(settings.general.deviceSettings[2].name, QString("Device 3 (TCP)"));
    QVERIFY(settings.general.deviceSettings[2].bConnectionId);
    QCOMPARE(settings.general.deviceSettings[2].connectionId, 1);
    QVERIFY(settings.general.deviceSettings[2].bSlaveId);
    QCOMPARE(settings.general.deviceSettings[2].slaveId, 1);
    QVERIFY(settings.general.deviceSettings[2].bConsecutiveMax);
    QCOMPARE(settings.general.deviceSettings[2].consecutiveMax, 125);
    QCOMPARE(settings.general.deviceSettings[2].bInt32LittleEndian, true);
}

void TestProjectFileParser::connEmpty()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    GeneralError parseError = projectParser.parseFile(ProjectFileTestData::cConnEmpty, &settings);
    QVERIFY(parseError.result());

    /* Connection id 0 */
    QCOMPARE(settings.general.connectionSettings.size(), 1);
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
    QVERIFY(settings.general.connectionSettings[0].bTimeout == false);
    QVERIFY(settings.general.connectionSettings[0].bPersistentConnection);

    /* Devices */
    QVERIFY(settings.general.deviceSettings.size() == 0);
}

void TestProjectFileParser::scaleDouble()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    GeneralError parseError = projectParser.parseFile(ProjectFileTestData::cScaleDouble, &settings);
    QVERIFY(parseError.result());

    /* Connection id 0 */
    QVERIFY(settings.view.scaleSettings.xAxis.bSliding);
    QCOMPARE(settings.view.scaleSettings.xAxis.slidingInterval, 20);

    QVERIFY(settings.view.scaleSettings.yAxis.bWindowScale == false);
    QVERIFY(settings.view.scaleSettings.yAxis.bMinMax);
    QCOMPARE(settings.view.scaleSettings.yAxis.scaleMin, 0);
    QCOMPARE(settings.view.scaleSettings.yAxis.scaleMax, 25.5);

    QVERIFY(settings.view.scaleSettings.y2Axis.bMinMax == false);
    QVERIFY(settings.view.scaleSettings.y2Axis.bWindowScale == false);
}

void TestProjectFileParser::valueAxis2Scaling()
{
    ProjectFileParser projectParser;
    ProjectFileData::ProjectSettings settings;

    GeneralError parseError = projectParser.parseFile(ProjectFileTestData::cValueAxis2Scaling, &settings);
    QVERIFY(parseError.result());

    /* Connection id 0 */
    QVERIFY(settings.view.scaleSettings.xAxis.bSliding);
    QCOMPARE(settings.view.scaleSettings.xAxis.slidingInterval, 20);

    QVERIFY(settings.view.scaleSettings.yAxis.bWindowScale);
    QVERIFY(settings.view.scaleSettings.yAxis.bMinMax == false);

    QVERIFY(settings.view.scaleSettings.y2Axis.bWindowScale == false);
    QVERIFY(settings.view.scaleSettings.y2Axis.bMinMax);
    QCOMPARE(settings.view.scaleSettings.y2Axis.scaleMin, 0);
    QCOMPARE(settings.view.scaleSettings.y2Axis.scaleMax, 25.5);
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
