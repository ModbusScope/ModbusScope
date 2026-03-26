
#include "tst_projectfilejsonparser.h"

#include "importexport/projectfilejsonparser.h"
#include "projectfilejsontestdata.h"

#include <QTest>

using ProjectFileData::ProjectSettings;

void TestProjectFileJsonParser::init()
{
}

void TestProjectFileJsonParser::initTestCase()
{
    QLocale::setDefault(QLocale(QLocale::Dutch, QLocale::Belgium));
}

void TestProjectFileJsonParser::cleanup()
{
}

void TestProjectFileJsonParser::tooLowVersion()
{
    ProjectFileJsonParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileJsonTestData::cTooLowVersion, &settings);
    QVERIFY(!err.result());
}

void TestProjectFileJsonParser::tooHighVersion()
{
    ProjectFileJsonParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileJsonTestData::cTooHighVersion, &settings);
    QVERIFY(!err.result());
}

void TestProjectFileJsonParser::missingVersion()
{
    ProjectFileJsonParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileJsonTestData::cMissingVersion, &settings);
    QVERIFY(!err.result());
}

void TestProjectFileJsonParser::adaptersAndDevices()
{
    ProjectFileJsonParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileJsonTestData::cAdaptersAndDevices, &settings);
    QVERIFY(err.result());

    /* Adapters */
    QCOMPARE(settings.general.adapterList.size(), 1);
    QCOMPARE(settings.general.adapterList[0].type, QString("modbus"));

    QJsonObject adapterSettings = settings.general.adapterList[0].settings;
    QVERIFY(adapterSettings.contains("connections"));
    QVERIFY(adapterSettings.contains("devices"));

    QJsonArray connections = adapterSettings["connections"].toArray();
    QCOMPARE(connections.size(), 1);

    QJsonObject conn0 = connections[0].toObject();
    QCOMPARE(conn0["id"].toInt(), 0);
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
    QVERIFY(conn0["enabled"].toBool());

    QJsonArray adapterDevices = adapterSettings["devices"].toArray();
    QCOMPARE(adapterDevices.size(), 1);
    QJsonObject dev0 = adapterDevices[0].toObject();
    QCOMPARE(dev0["id"].toInt(), 0);
    QCOMPARE(dev0["connectionid"].toInt(), 0);
    QCOMPARE(dev0["slaveid"].toInt(), 3);
    QCOMPARE(dev0["consecutivemax"].toInt(), 100);
    QVERIFY(!dev0["int32littleendian"].toBool());

    /* Generic devices */
    QCOMPARE(settings.general.deviceSettings.size(), 1);
    QCOMPARE(settings.general.deviceSettings[0].deviceId, static_cast<deviceId_t>(1));
    QVERIFY(settings.general.deviceSettings[0].bDeviceId);
    QCOMPARE(settings.general.deviceSettings[0].adapterId, static_cast<quint32>(0));
    QCOMPARE(settings.general.deviceSettings[0].adapterType, QString("modbus"));
    QVERIFY(settings.general.deviceSettings[0].bName);
    QCOMPARE(settings.general.deviceSettings[0].name, QString("Device 1"));

    /* Log */
    QVERIFY(settings.general.logSettings.bPollTime);
    QCOMPARE(settings.general.logSettings.pollTime, static_cast<quint32>(250));
    QVERIFY(!settings.general.logSettings.bAbsoluteTimes);
    QVERIFY(!settings.general.logSettings.bLogToFile);
}

void TestProjectFileJsonParser::multipleAdaptersAndDevices()
{
    ProjectFileJsonParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileJsonTestData::cMultipleAdaptersAndDevices, &settings);
    QVERIFY(err.result());

    QCOMPARE(settings.general.adapterList.size(), 2);
    QCOMPARE(settings.general.adapterList[0].type, QString("modbus"));
    QCOMPARE(settings.general.adapterList[1].type, QString("custom"));
    QVERIFY(settings.general.adapterList[1].settings.contains("endpoint"));

    QCOMPARE(settings.general.deviceSettings.size(), 2);

    QCOMPARE(settings.general.deviceSettings[0].deviceId, static_cast<deviceId_t>(1));
    QCOMPARE(settings.general.deviceSettings[0].adapterId, static_cast<quint32>(0));
    QCOMPARE(settings.general.deviceSettings[0].adapterType, QString("modbus"));
    QCOMPARE(settings.general.deviceSettings[0].name, QString("Modbus Device"));

    QCOMPARE(settings.general.deviceSettings[1].deviceId, static_cast<deviceId_t>(2));
    QCOMPARE(settings.general.deviceSettings[1].adapterId, static_cast<quint32>(1));
    QCOMPARE(settings.general.deviceSettings[1].adapterType, QString("custom"));
    QCOMPARE(settings.general.deviceSettings[1].name, QString("Custom Device"));

    QVERIFY(settings.general.logSettings.bPollTime);
    QCOMPARE(settings.general.logSettings.pollTime, static_cast<quint32>(500));
    QVERIFY(settings.general.logSettings.bAbsoluteTimes);
}

void TestProjectFileJsonParser::adapterOnlyNoDevices()
{
    ProjectFileJsonParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileJsonTestData::cAdapterOnlyNoDevices, &settings);
    QVERIFY(err.result());

    QCOMPARE(settings.general.adapterList.size(), 1);
    QCOMPARE(settings.general.adapterList[0].type, QString("modbus"));
    QCOMPARE(settings.general.deviceSettings.size(), 0);
}

void TestProjectFileJsonParser::logSettings()
{
    ProjectFileJsonParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileJsonTestData::cLogSettings, &settings);
    QVERIFY(err.result());

    QVERIFY(settings.general.logSettings.bPollTime);
    QCOMPARE(settings.general.logSettings.pollTime, static_cast<quint32>(750));
    QVERIFY(settings.general.logSettings.bAbsoluteTimes);
    QVERIFY(settings.general.logSettings.bLogToFile);
    QVERIFY(!settings.general.logSettings.bLogToFileFile);
}

void TestProjectFileJsonParser::logToFileWithPath()
{
    ProjectFileJsonParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileJsonTestData::cLogToFileWithPath, &settings);
    /* Result depends on whether /tmp/test_mbs_log.csv parent dir exists; just check no crash */
    Q_UNUSED(err)
    QVERIFY(settings.general.logSettings.bLogToFile);
}

void TestProjectFileJsonParser::scopeRegisters()
{
    ProjectFileJsonParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileJsonTestData::cScopeRegisters, &settings);
    QVERIFY(err.result());

    QCOMPARE(settings.scope.registerList.size(), 3);

    QVERIFY(settings.scope.registerList[0].bActive);
    QCOMPARE(settings.scope.registerList[0].text, QString("Data point"));
    QCOMPARE(settings.scope.registerList[0].expression, QString("${40001}/2"));
    QVERIFY(settings.scope.registerList[0].bColor);
    QCOMPARE(settings.scope.registerList[0].color, QColor(0xff, 0x00, 0x00));
    QCOMPARE(settings.scope.registerList[0].valueAxis, static_cast<quint32>(0));

    QVERIFY(settings.scope.registerList[1].bActive);
    QCOMPARE(settings.scope.registerList[1].text, QString("Data point 2"));
    QCOMPARE(settings.scope.registerList[1].expression, QString("${40002:s16b}"));
    QVERIFY(settings.scope.registerList[1].bColor);
    QCOMPARE(settings.scope.registerList[1].color, QColor(0x00, 0x00, 0xff));
    QCOMPARE(settings.scope.registerList[1].valueAxis, static_cast<quint32>(1));

    QVERIFY(!settings.scope.registerList[2].bActive);
    QCOMPARE(settings.scope.registerList[2].text, QString("Data point 3"));
    QCOMPARE(settings.scope.registerList[2].expression, QString("${40003:s16b}*10"));
    QVERIFY(settings.scope.registerList[2].bColor);
    QCOMPARE(settings.scope.registerList[2].color, QColor(0x00, 0xff, 0xff));
}

void TestProjectFileJsonParser::viewScaleSliding()
{
    ProjectFileJsonParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileJsonTestData::cViewScaleSliding, &settings);
    QVERIFY(err.result());

    QVERIFY(settings.view.scaleSettings.xAxis.bSliding);
    QCOMPARE(settings.view.scaleSettings.xAxis.slidingInterval, static_cast<quint32>(30));
}

void TestProjectFileJsonParser::viewScaleMinMax()
{
    ProjectFileJsonParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileJsonTestData::cViewScaleMinMax, &settings);
    QVERIFY(err.result());

    QVERIFY(!settings.view.scaleSettings.xAxis.bSliding);

    QVERIFY(settings.view.scaleSettings.yAxis.bMinMax);
    QVERIFY(!settings.view.scaleSettings.yAxis.bWindowScale);
    QCOMPARE(settings.view.scaleSettings.yAxis.scaleMin, -5.5);
    QCOMPARE(settings.view.scaleSettings.yAxis.scaleMax, 100.25);

    QVERIFY(!settings.view.scaleSettings.y2Axis.bMinMax);
    QVERIFY(settings.view.scaleSettings.y2Axis.bWindowScale);
}

void TestProjectFileJsonParser::viewScaleWindowAuto()
{
    ProjectFileJsonParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileJsonTestData::cViewScaleWindowAuto, &settings);
    QVERIFY(err.result());

    QVERIFY(settings.view.scaleSettings.yAxis.bWindowScale);
    QVERIFY(!settings.view.scaleSettings.yAxis.bMinMax);
}

void TestProjectFileJsonParser::viewScaleAuto()
{
    ProjectFileJsonParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileJsonTestData::cViewScaleAuto, &settings);
    QVERIFY(err.result());

    QVERIFY(!settings.view.scaleSettings.xAxis.bSliding);
    QVERIFY(!settings.view.scaleSettings.yAxis.bMinMax);
    QVERIFY(!settings.view.scaleSettings.yAxis.bWindowScale);
    QVERIFY(!settings.view.scaleSettings.y2Axis.bMinMax);
    QVERIFY(!settings.view.scaleSettings.y2Axis.bWindowScale);
}

QTEST_MAIN(TestProjectFileJsonParser)
