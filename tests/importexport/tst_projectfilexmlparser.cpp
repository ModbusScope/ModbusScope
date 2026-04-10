
#include "tst_projectfilexmlparser.h"

#include "importexport/projectfilexmlparser.h"
#include "projectfilexmltestdata.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QLocale>
#include <QTest>

using ProjectFileData::ProjectSettings;

void TestProjectFileXmlParser::init()
{
}

void TestProjectFileXmlParser::initTestCase()
{
    QLocale::setDefault(QLocale(QLocale::Dutch, QLocale::Belgium));
}

void TestProjectFileXmlParser::cleanup()
{
}

void TestProjectFileXmlParser::tooLowDataLevel()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileXmlTestData::cTooLowDataLevel, &settings);
    QVERIFY(!err.result());
}

void TestProjectFileXmlParser::tooHighDataLevel()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileXmlTestData::cTooHighDataLevel, &settings);
    QVERIFY(!err.result());
}

void TestProjectFileXmlParser::dataLevel3Expressions()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileXmlTestData::cDataLevel3Expressions, &settings);
    QVERIFY(err.result());

    QCOMPARE(settings.scope.registerList.size(), 3);

    QVERIFY(settings.scope.registerList[0].bActive);
    QCOMPARE(settings.scope.registerList[0].text, QString("Data point"));
    QCOMPARE(settings.scope.registerList[0].expression, QString("${40001}/2"));
    QVERIFY(settings.scope.registerList[0].bColor);
    QCOMPARE(settings.scope.registerList[0].color, QColor("#ff0000"));

    QVERIFY(settings.scope.registerList[1].bActive);
    QCOMPARE(settings.scope.registerList[1].text, QString("Data point 2"));
    QCOMPARE(settings.scope.registerList[1].expression, QString("${40002:s16b}"));
    QVERIFY(settings.scope.registerList[1].bColor);
    QCOMPARE(settings.scope.registerList[1].color, QColor("#0000ff"));

    QVERIFY(!settings.scope.registerList[2].bActive);
    QCOMPARE(settings.scope.registerList[2].text, QString("Data point 3"));
    QCOMPARE(settings.scope.registerList[2].expression, QString("${40003@2:s16b}*10"));
}

void TestProjectFileXmlParser::dataLevel5Connection()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileXmlTestData::cDataLevel5Connection, &settings);
    QVERIFY(err.result());

    /* Adapter blob */
    QCOMPARE(settings.general.adapterList.size(), 1);
    QCOMPARE(settings.general.adapterList[0].type, QString("modbus"));

    QJsonObject adapterSettings = settings.general.adapterList[0].settings;
    QVERIFY(adapterSettings.contains("connections"));
    QVERIFY(adapterSettings.contains("devices"));

    QJsonArray connections = adapterSettings["connections"].toArray();
    QCOMPARE(connections.size(), 1);

    QJsonObject conn0 = connections[0].toObject();
    QCOMPARE(conn0["id"].toInt(), 0);
    QCOMPARE(conn0["enabled"].toBool(), true);
    QCOMPARE(conn0["connectiontype"].toString(), QString("tcp"));
    QCOMPARE(conn0["ip"].toString(), QString("127.0.0.1"));
    QCOMPARE(conn0["port"].toInt(), 502);
    QCOMPARE(conn0["portname"].toString(), QString("COM1"));
    QCOMPARE(conn0["baudrate"].toInt(), 115200);
    QCOMPARE(conn0["parity"].toInt(), 0);
    QCOMPARE(conn0["stopbits"].toInt(), 1);
    QCOMPARE(conn0["databits"].toInt(), 8);
    QCOMPARE(conn0["timeout"].toInt(), 1000);
    QVERIFY(conn0["persistentconnection"].toBool());

    QJsonArray adapterDevices = adapterSettings["devices"].toArray();
    QCOMPARE(adapterDevices.size(), 1);
    QJsonObject dev0 = adapterDevices[0].toObject();
    QCOMPARE(dev0["id"].toInt(), 1);
    QCOMPARE(dev0["connectionid"].toInt(), 0);
    QCOMPARE(dev0["slaveid"].toInt(), 3);
    QCOMPARE(dev0["consecutivemax"].toInt(), 100);
    QVERIFY(!dev0["int32littleendian"].toBool());

    /* Generic devices */
    QCOMPARE(settings.general.deviceSettings.size(), 1);
    QVERIFY(settings.general.deviceSettings[0].bDeviceId);
    QCOMPARE(settings.general.deviceSettings[0].deviceId, static_cast<deviceId_t>(1));
    QCOMPARE(settings.general.deviceSettings[0].adapterId, static_cast<quint32>(0));
    QCOMPARE(settings.general.deviceSettings[0].adapterType, QString("modbus"));
    QVERIFY(settings.general.deviceSettings[0].bName);
    QCOMPARE(settings.general.deviceSettings[0].name, QString("Device 1"));

    /* Log */
    QVERIFY(settings.general.logSettings.bPollTime);
    QCOMPARE(settings.general.logSettings.pollTime, static_cast<quint32>(250));
    QVERIFY(!settings.general.logSettings.bAbsoluteTimes);
    QVERIFY(settings.general.logSettings.bLogToFile);
}

void TestProjectFileXmlParser::dataLevel5MixedMulti()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileXmlTestData::cDataLevel5MixedMulti, &settings);
    QVERIFY(err.result());

    /* 3 connections, 3 devices */
    QJsonObject adapterSettings = settings.general.adapterList[0].settings;
    QJsonArray connections = adapterSettings["connections"].toArray();
    QCOMPARE(connections.size(), 3);

    /* First connection: serial */
    QJsonObject conn0 = connections[0].toObject();
    QCOMPARE(conn0["connectiontype"].toString(), QString("serial"));
    QCOMPARE(conn0["portname"].toString(), QString("COM10"));
    QCOMPARE(conn0["baudrate"].toInt(), 38400);
    QCOMPARE(conn0["timeout"].toInt(), 500);

    /* Third connection: disabled */
    QJsonObject conn2 = connections[2].toObject();
    QVERIFY(!conn2["enabled"].toBool());

    /* 3 generic devices */
    QCOMPARE(settings.general.deviceSettings.size(), 3);
    QCOMPARE(settings.general.deviceSettings[0].deviceId, static_cast<deviceId_t>(1));
    QCOMPARE(settings.general.deviceSettings[0].name, QString("Device 1 (serial 1)"));
    QCOMPARE(settings.general.deviceSettings[1].deviceId, static_cast<deviceId_t>(2));
    QCOMPARE(settings.general.deviceSettings[1].name, QString("Device 2 (serial 2)"));
    QCOMPARE(settings.general.deviceSettings[2].deviceId, static_cast<deviceId_t>(3));
    QCOMPARE(settings.general.deviceSettings[2].name, QString("Device 3 (TCP)"));

    /* Adapter devices */
    QJsonArray adapterDevices = adapterSettings["devices"].toArray();
    QCOMPARE(adapterDevices.size(), 3);
    QCOMPARE(adapterDevices[0].toObject()["connectionid"].toInt(), 0);
    QCOMPARE(adapterDevices[0].toObject()["slaveid"].toInt(), 1);
    QCOMPARE(adapterDevices[2].toObject()["connectionid"].toInt(), 1);
}

void TestProjectFileXmlParser::dataLevel4LegacyConnection()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileXmlTestData::cDataLevel4LegacyConnection, &settings);
    QVERIFY(err.result());

    /* Only enabled connections create devices in legacy mode */
    QCOMPARE(settings.general.deviceSettings.size(), 1);
    QVERIFY(settings.general.deviceSettings[0].bDeviceId);
    QCOMPARE(settings.general.deviceSettings[0].deviceId, static_cast<deviceId_t>(1));

    /* Adapter blob should have both connections */
    QJsonObject adapterSettings = settings.general.adapterList[0].settings;
    QJsonArray connections = adapterSettings["connections"].toArray();
    QCOMPARE(connections.size(), 2);

    /* First connection TCP */
    QJsonObject conn0 = connections[0].toObject();
    QCOMPARE(conn0["connectiontype"].toString(), QString("tcp"));
    QCOMPARE(conn0["timeout"].toInt(), 1002);

    /* Only one adapter device (from the enabled connection) */
    QJsonArray adapterDevices = adapterSettings["devices"].toArray();
    QCOMPARE(adapterDevices.size(), 1);
    QCOMPARE(adapterDevices[0].toObject()["slaveid"].toInt(), 2);
    QCOMPARE(adapterDevices[0].toObject()["consecutivemax"].toInt(), 122);
    QVERIFY(adapterDevices[0].toObject()["int32littleendian"].toBool());
}

void TestProjectFileXmlParser::logSettings()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileXmlTestData::cLogSettings, &settings);
    QVERIFY(err.result());

    QVERIFY(settings.general.logSettings.bPollTime);
    QCOMPARE(settings.general.logSettings.pollTime, static_cast<quint32>(750));
    QVERIFY(settings.general.logSettings.bAbsoluteTimes);
    QVERIFY(!settings.general.logSettings.bLogToFile);
}

void TestProjectFileXmlParser::scaleSliding()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileXmlTestData::cScaleSliding, &settings);
    QVERIFY(err.result());

    QVERIFY(settings.view.scaleSettings.xAxis.bSliding);
    QCOMPARE(settings.view.scaleSettings.xAxis.slidingInterval, static_cast<quint32>(20));
}

void TestProjectFileXmlParser::scaleMinMax()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileXmlTestData::cScaleMinMax, &settings);
    QVERIFY(err.result());

    QVERIFY(settings.view.scaleSettings.xAxis.bSliding);
    QCOMPARE(settings.view.scaleSettings.xAxis.slidingInterval, static_cast<quint32>(20));

    QVERIFY(settings.view.scaleSettings.yAxis.bMinMax);
    QCOMPARE(settings.view.scaleSettings.yAxis.scaleMin, 0.0);
    QCOMPARE(settings.view.scaleSettings.yAxis.scaleMax, 25.5);
}

void TestProjectFileXmlParser::scaleWindowAuto()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileXmlTestData::cScaleWindowAuto, &settings);
    QVERIFY(err.result());

    QVERIFY(settings.view.scaleSettings.yAxis.bWindowScale);
    QVERIFY(!settings.view.scaleSettings.yAxis.bMinMax);

    QVERIFY(settings.view.scaleSettings.y2Axis.bMinMax);
    QCOMPARE(settings.view.scaleSettings.y2Axis.scaleMin, 0.0);
    QCOMPARE(settings.view.scaleSettings.y2Axis.scaleMax, 25.5);
}

void TestProjectFileXmlParser::valueAxis()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileXmlTestData::cValueAxis, &settings);
    QVERIFY(err.result());

    QCOMPARE(settings.scope.registerList.size(), 3);

    QCOMPARE(settings.scope.registerList[0].valueAxis, static_cast<quint32>(0));
    QCOMPARE(settings.scope.registerList[1].valueAxis, static_cast<quint32>(1));
    QCOMPARE(settings.scope.registerList[2].valueAxis, static_cast<quint32>(0));
}

QTEST_MAIN(TestProjectFileXmlParser)

#include "tst_projectfilexmlparser.moc"
