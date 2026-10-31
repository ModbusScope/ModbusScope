
#include "tst_projectfilexmlparser.h"

#include "importexport/legacy/projectfilexmlparser.h"
#include "projectfilexmltestdata.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QLocale>
#include <QTemporaryDir>
#include <QTest>

#include <utility>

using ProjectFileData::ProjectSettings;

namespace {

//! Build a datalevel 5 file with one connection made of \a connectionTags and one device on it.
QString singleConnectionFile(const QString& connectionTags)
{
    return QString("<?xml version=\"1.0\"?>\n"
                   "<modbusscope datalevel=\"5\">\n"
                   " <modbus>\n"
                   "  <connection>\n"
                   "   <connectionid>0</connectionid>\n"
                   "%1"
                   "  </connection>\n"
                   "  <device>\n"
                   "   <deviceid>1</deviceid>\n"
                   "   <connectionid>0</connectionid>\n"
                   "  </device>\n"
                   " </modbus>\n"
                   "</modbusscope>\n")
      .arg(connectionTags);
}

QJsonArray adapterConnections(const ProjectSettings& settings)
{
    return settings.general.adapterList.at(0).settings.value("connections").toArray();
}

QJsonArray adapterDevices(const ProjectSettings& settings)
{
    return settings.general.adapterList.at(0).settings.value("devices").toArray();
}

//! Return the ids of all connections in \a connections.
QList<int> connectionIds(const QJsonArray& connections)
{
    QList<int> ids;
    for (const auto& connection : connections)
    {
        ids.append(connection.toObject().value("id").toInt(-1));
    }
    return ids;
}

const QStringList cLegacyConnectionKeys = { "connectiontype", "portname", "persistentconnection", "enabled" };
const QStringList cLegacyDeviceKeys = { "connectionid", "slaveid", "consecutivemax", "int32littleendian" };

} // namespace

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
    QCOMPARE(conn0["id"].toInt(), 1);
    QCOMPARE(conn0["name"].toString(), QString("Connection 1"));
    QVERIFY(!conn0.contains("enabled"));
    QCOMPARE(conn0["type"].toString(), QString("tcp"));
    QCOMPARE(conn0["ip"].toString(), QString("127.0.0.1"));
    QCOMPARE(conn0["port"].toInt(), 502);
    QCOMPARE(conn0["portName"].toString(), QString("COM1"));
    QCOMPARE(conn0["baudrate"].toInt(), 115200);
    QCOMPARE(conn0["parity"].toString(), QString("N"));
    QCOMPARE(conn0["stopbits"].toInt(), 1);
    QCOMPARE(conn0["databits"].toInt(), 8);
    QCOMPARE(conn0["timeout"].toInt(), 1000);
    QVERIFY(conn0["persistent"].toBool());

    QJsonArray adapterDevices = adapterSettings["devices"].toArray();
    QCOMPARE(adapterDevices.size(), 1);
    QJsonObject dev0 = adapterDevices[0].toObject();
    QCOMPARE(dev0["id"].toInt(), 1);
    QCOMPARE(dev0["connectionId"].toInt(), 1);
    QCOMPARE(dev0["slaveId"].toInt(), 3);
    QCOMPARE(dev0["consecutiveMax"].toInt(), 100);
    QVERIFY(!dev0["int32LittleEndian"].toBool());

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

    /* The third connection is disabled and unused, so only 2 connections remain */
    QJsonObject adapterSettings = settings.general.adapterList[0].settings;
    QJsonArray connections = adapterSettings["connections"].toArray();
    QCOMPARE(connections.size(), 2);

    /* First connection: serial */
    QJsonObject conn0 = connections[0].toObject();
    QCOMPARE(conn0["id"].toInt(), 1);
    QCOMPARE(conn0["type"].toString(), QString("serial"));
    QCOMPARE(conn0["portName"].toString(), QString("COM10"));
    QCOMPARE(conn0["baudrate"].toInt(), 38400);
    QCOMPARE(conn0["timeout"].toInt(), 500);

    QJsonObject conn1 = connections[1].toObject();
    QCOMPARE(conn1["id"].toInt(), 2);
    QCOMPARE(conn1["type"].toString(), QString("tcp"));

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
    QCOMPARE(adapterDevices[0].toObject()["connectionId"].toInt(), 1);
    QCOMPARE(adapterDevices[0].toObject()["slaveId"].toInt(), 1);
    QCOMPARE(adapterDevices[1].toObject()["connectionId"].toInt(), 1);
    QCOMPARE(adapterDevices[1].toObject()["slaveId"].toInt(), 2);
    QCOMPARE(adapterDevices[2].toObject()["connectionId"].toInt(), 2);
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

    /* The disabled connection has no device, so only the enabled one remains */
    QJsonObject adapterSettings = settings.general.adapterList[0].settings;
    QJsonArray connections = adapterSettings["connections"].toArray();
    QCOMPARE(connections.size(), 1);

    QJsonObject conn0 = connections[0].toObject();
    QCOMPARE(conn0["id"].toInt(), 1);
    QCOMPARE(conn0["type"].toString(), QString("tcp"));
    QCOMPARE(conn0["timeout"].toInt(), 1002);

    /* Only one adapter device (from the enabled connection) */
    QJsonArray adapterDevices = adapterSettings["devices"].toArray();
    QCOMPARE(adapterDevices.size(), 1);
    QCOMPARE(adapterDevices[0].toObject()["connectionId"].toInt(), 1);
    QCOMPARE(adapterDevices[0].toObject()["slaveId"].toInt(), 2);
    QCOMPARE(adapterDevices[0].toObject()["consecutiveMax"].toInt(), 122);
    QVERIFY(adapterDevices[0].toObject()["int32LittleEndian"].toBool());
}

/*!
 * \brief Two legacy connections omitting <connectionid> both synthesise device id 1.
 *
 * This is the root of the duplicate device id that ProjectFileHandler::buildDeviceList()
 * merges and AdapterData::configForWire() drops: the id is derived from <connectionid>, which
 * defaults to 0 when the tag is absent. The parser must keep accepting such a file - it is
 * otherwise valid, and refusing it would stop a project that has always opened from opening.
 */
void TestProjectFileXmlParser::dataLevel4LegacyConnectionsWithoutConnectionIdCollide()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileXmlTestData::cDataLevel4LegacyConnectionNoIds, &settings);
    QVERIFY(err.result());

    QCOMPARE(settings.general.deviceSettings.size(), 2);
    for (const ProjectFileData::DeviceSettings& device : std::as_const(settings.general.deviceSettings))
    {
        QVERIFY(device.bDeviceId);
        QCOMPARE(device.deviceId, static_cast<deviceId_t>(1));
        QVERIFY(!device.bName);
    }

    /* The same collision reaches the adapter blob, which is why configForWire() dedupes. */
    QJsonArray adapterDevices = settings.general.adapterList[0].settings["devices"].toArray();
    QCOMPARE(adapterDevices.size(), 2);
    QCOMPARE(adapterDevices[0].toObject().value("id").toInt(), 1);
    QCOMPARE(adapterDevices[1].toObject().value("id").toInt(), 1);
    QCOMPARE(adapterDevices[0].toObject().value("connectionId").toInt(), 1);
    QCOMPARE(adapterDevices[1].toObject().value("connectionId").toInt(), 1);

    /* Both connections also collide on id 1; the first one is kept. */
    QJsonArray connections = settings.general.adapterList[0].settings["connections"].toArray();
    QCOMPARE(connections.size(), 1);
    QCOMPARE(connections[0].toObject().value("id").toInt(), 1);
    QCOMPARE(connections[0].toObject().value("ip").toString(), QString("127.0.0.1"));
}

void TestProjectFileXmlParser::connectionUsesAdapterKeys_data()
{
    QTest::addColumn<QString>("fileContent");

    QTest::newRow("datalevel 3") << ProjectFileXmlTestData::cDataLevel3Connections;
    QTest::newRow("datalevel 4 demo") << ProjectFileXmlTestData::cDataLevel4Demo;
    QTest::newRow("datalevel 5 demo") << ProjectFileXmlTestData::cDataLevel5Demo;
    QTest::newRow("datalevel 5 mixed") << ProjectFileXmlTestData::cDataLevel5MixedMulti;
    QTest::newRow("serial variants") << ProjectFileXmlTestData::cSerialVariants;
    QTest::newRow("minimal connection") << ProjectFileXmlTestData::cMinimalConnection;
}

void TestProjectFileXmlParser::connectionUsesAdapterKeys()
{
    QFETCH(QString, fileContent);

    ProjectFileXmlParser parser;
    ProjectSettings settings;
    QVERIFY(parser.parseFile(fileContent, &settings).result());

    const QJsonArray connections = adapterConnections(settings);
    QVERIFY(!connections.isEmpty());
    for (const auto& connection : connections)
    {
        const QJsonObject obj = connection.toObject();
        for (const QString& key : cLegacyConnectionKeys)
        {
            QVERIFY2(!obj.contains(key), qPrintable(QString("connection has legacy key %1").arg(key)));
        }
        const QStringList requiredKeys = { "id",       "name",   "type",     "ip",       "port",    "portName",
                                           "baudrate", "parity", "stopbits", "databits", "timeout", "persistent" };
        for (const QString& key : requiredKeys)
        {
            QVERIFY2(obj.contains(key), qPrintable(QString("connection misses key %1").arg(key)));
        }
    }

    const QJsonArray devices = adapterDevices(settings);
    QVERIFY(!devices.isEmpty());
    for (const auto& device : devices)
    {
        const QJsonObject obj = device.toObject();
        for (const QString& key : cLegacyDeviceKeys)
        {
            QVERIFY2(!obj.contains(key), qPrintable(QString("device has legacy key %1").arg(key)));
        }
        const QStringList requiredKeys = { "id", "connectionId", "slaveId", "consecutiveMax", "int32LittleEndian" };
        for (const QString& key : requiredKeys)
        {
            QVERIFY2(obj.contains(key), qPrintable(QString("device misses key %1").arg(key)));
        }
    }
}

void TestProjectFileXmlParser::deviceConnectionIdMatchesConnection_data()
{
    connectionUsesAdapterKeys_data();
    QTest::newRow("disabled referenced") << ProjectFileXmlTestData::cDisabledReferencedConnection;
}

void TestProjectFileXmlParser::deviceConnectionIdMatchesConnection()
{
    QFETCH(QString, fileContent);

    ProjectFileXmlParser parser;
    ProjectSettings settings;
    QVERIFY(parser.parseFile(fileContent, &settings).result());

    const QList<int> ids = connectionIds(adapterConnections(settings));
    for (int id : ids)
    {
        QVERIFY2(id >= 1, qPrintable(QString("connection id %1 is below 1").arg(id)));
    }

    const QJsonArray devices = adapterDevices(settings);
    for (const auto& device : devices)
    {
        const int connectionId = device.toObject().value("connectionId").toInt(-1);
        QVERIFY2(ids.contains(connectionId),
                 qPrintable(QString("device references unknown connection %1").arg(connectionId)));
    }
}

void TestProjectFileXmlParser::connectionIdsStartAtOne()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;
    QVERIFY(parser.parseFile(ProjectFileXmlTestData::cDataLevel3Connections, &settings).result());

    /* Connections 0 and 1 are enabled, connection 2 is disabled and unused */
    QCOMPARE(connectionIds(adapterConnections(settings)), QList<int>({ 1, 2 }));

    const QJsonArray devices = adapterDevices(settings);
    QCOMPARE(devices.size(), 2);
    QCOMPARE(devices[0].toObject().value("connectionId").toInt(), 1);
    QCOMPARE(devices[1].toObject().value("connectionId").toInt(), 2);
}

void TestProjectFileXmlParser::parityMapsToLetters_data()
{
    QTest::addColumn<QString>("parityTag");
    QTest::addColumn<QString>("expected");

    QTest::newRow("none") << "0" << "N";
    QTest::newRow("even") << "2" << "E";
    QTest::newRow("odd") << "3" << "O";
    QTest::newRow("space") << "4" << "N";
    QTest::newRow("mark") << "5" << "N";
}

void TestProjectFileXmlParser::parityMapsToLetters()
{
    QFETCH(QString, parityTag);
    QFETCH(QString, expected);

    const QString file =
      singleConnectionFile(QString("   <type>serial</type>\n   <parity>%1</parity>\n").arg(parityTag));

    ProjectFileXmlParser parser;
    ProjectSettings settings;
    QVERIFY(parser.parseFile(file, &settings).result());

    QCOMPARE(adapterConnections(settings)[0].toObject().value("parity").toString(), expected);
}

void TestProjectFileXmlParser::parityMissingDefaultsToNone()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;
    QVERIFY(parser.parseFile(singleConnectionFile("   <type>serial</type>\n"), &settings).result());

    QCOMPARE(adapterConnections(settings)[0].toObject().value("parity").toString(), QString("N"));
}

void TestProjectFileXmlParser::invalidNumberIsParseError_data()
{
    QTest::addColumn<QString>("connectionTags");

    QTest::newRow("parity") << "   <parity>none</parity>\n";
    QTest::newRow("connectionid") << "   <connectionid>x</connectionid>\n";
    QTest::newRow("baudrate") << "   <baudrate>fast</baudrate>\n";
    QTest::newRow("port") << "   <port>http</port>\n";
    QTest::newRow("stopbits") << "   <stopbits>one</stopbits>\n";
    QTest::newRow("databits") << "   <databits>eight</databits>\n";
    QTest::newRow("timeout") << "   <timeout>-</timeout>\n";
}

void TestProjectFileXmlParser::invalidNumberIsParseError()
{
    QFETCH(QString, connectionTags);

    ProjectFileXmlParser parser;
    ProjectSettings settings;
    QVERIFY(!parser.parseFile(singleConnectionFile(connectionTags), &settings).result());
}

void TestProjectFileXmlParser::stopbitsAndDatabitsKept()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;
    QVERIFY(parser.parseFile(ProjectFileXmlTestData::cSerialVariants, &settings).result());

    const QJsonArray connections = adapterConnections(settings);
    QCOMPARE(connections.size(), 2);

    const QJsonObject conn0 = connections[0].toObject();
    QCOMPARE(conn0.value("type").toString(), QString("serial"));
    QCOMPARE(conn0.value("portName").toString(), QString("COM4"));
    QCOMPARE(conn0.value("baudrate").toInt(), 9600);
    QCOMPARE(conn0.value("parity").toString(), QString("E"));
    QCOMPARE(conn0.value("stopbits").toInt(), 3);
    QCOMPARE(conn0.value("databits").toInt(), 7);

    const QJsonObject conn1 = connections[1].toObject();
    QCOMPARE(conn1.value("portName").toString(), QString("/dev/ttyUSB0"));
    QCOMPARE(conn1.value("baudrate").toInt(), 1200);
    QCOMPARE(conn1.value("parity").toString(), QString("O"));
    QCOMPARE(conn1.value("stopbits").toInt(), 2);
    QCOMPARE(conn1.value("databits").toInt(), 5);
    QVERIFY(!conn1.value("persistent").toBool());
}

void TestProjectFileXmlParser::outOfRangeValuesUseDefaults()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;
    QVERIFY(parser.parseFile(ProjectFileXmlTestData::cOutOfRangeValues, &settings).result());

    const QJsonObject conn0 = adapterConnections(settings)[0].toObject();
    QCOMPARE(conn0.value("baudrate").toInt(), 115200);
    QCOMPARE(conn0.value("parity").toString(), QString("N"));
    QCOMPARE(conn0.value("stopbits").toInt(), 1);
    QCOMPARE(conn0.value("databits").toInt(), 8);
    QCOMPARE(conn0.value("port").toInt(), 502);
}

void TestProjectFileXmlParser::outOfRangeDeviceValuesUseDefaults_data()
{
    QTest::addColumn<QString>("fileContent");
    QTest::addColumn<int>("expectedSlaveId");
    QTest::addColumn<int>("expectedConsecutiveMax");

    const QString legacyFile("<?xml version=\"1.0\"?>\n"
                             "<modbusscope datalevel=\"4\">\n"
                             " <modbus>\n"
                             "  <connection>\n"
                             "   <slaveid>%1</slaveid>\n"
                             "   <consecutivemax>%2</consecutivemax>\n"
                             "  </connection>\n"
                             " </modbus>\n"
                             "</modbusscope>\n");
    const QString deviceFile("<?xml version=\"1.0\"?>\n"
                             "<modbusscope datalevel=\"5\">\n"
                             " <modbus>\n"
                             "  <connection>\n"
                             "   <connectionid>0</connectionid>\n"
                             "  </connection>\n"
                             "  <device>\n"
                             "   <deviceid>1</deviceid>\n"
                             "   <connectionid>0</connectionid>\n"
                             "   <slaveid>%1</slaveid>\n"
                             "   <consecutivemax>%2</consecutivemax>\n"
                             "  </device>\n"
                             " </modbus>\n"
                             "</modbusscope>\n");

    QTest::newRow("datalevel 4 zero") << legacyFile.arg(0).arg(0) << 1 << 125;
    QTest::newRow("datalevel 4 above maximum") << legacyFile.arg(248).arg(126) << 1 << 125;
    QTest::newRow("datalevel 4 above 255") << legacyFile.arg(300).arg(300) << 1 << 125;
    QTest::newRow("datalevel 4 limits") << legacyFile.arg(247).arg(1) << 247 << 1;
    QTest::newRow("datalevel 5 zero") << deviceFile.arg(0).arg(0) << 1 << 125;
    QTest::newRow("datalevel 5 above 255") << deviceFile.arg(300).arg(300) << 1 << 125;
    QTest::newRow("datalevel 5 limits") << deviceFile.arg(1).arg(125) << 1 << 125;
}

void TestProjectFileXmlParser::outOfRangeDeviceValuesUseDefaults()
{
    QFETCH(QString, fileContent);
    QFETCH(int, expectedSlaveId);
    QFETCH(int, expectedConsecutiveMax);

    ProjectFileXmlParser parser;
    ProjectSettings settings;
    QVERIFY(parser.parseFile(fileContent, &settings).result());

    const QJsonObject dev0 = adapterDevices(settings)[0].toObject();
    QCOMPARE(dev0.value("slaveId").toInt(), expectedSlaveId);
    QCOMPARE(dev0.value("consecutiveMax").toInt(), expectedConsecutiveMax);
}

void TestProjectFileXmlParser::typeDefaultsToTcp_data()
{
    QTest::addColumn<QString>("connectionTags");
    QTest::addColumn<QString>("expected");

    QTest::newRow("missing") << "" << "tcp";
    QTest::newRow("tcp") << "   <type>tcp</type>\n" << "tcp";
    QTest::newRow("TCP") << "   <type>TCP</type>\n" << "tcp";
    QTest::newRow("unknown") << "   <type>usb</type>\n" << "tcp";
    QTest::newRow("serial") << "   <type>serial</type>\n" << "serial";
    QTest::newRow("Serial") << "   <type>Serial</type>\n" << "serial";
}

void TestProjectFileXmlParser::typeDefaultsToTcp()
{
    QFETCH(QString, connectionTags);
    QFETCH(QString, expected);

    ProjectFileXmlParser parser;
    ProjectSettings settings;
    QVERIFY(parser.parseFile(singleConnectionFile(connectionTags), &settings).result());

    QCOMPARE(adapterConnections(settings)[0].toObject().value("type").toString(), expected);
}

void TestProjectFileXmlParser::missingFieldsUseOldDefaults()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;
    QVERIFY(parser.parseFile(ProjectFileXmlTestData::cMinimalConnection, &settings).result());

    const QJsonArray connections = adapterConnections(settings);
    QCOMPARE(connections.size(), 1);
    const QJsonObject conn0 = connections[0].toObject();
    QCOMPARE(conn0.value("id").toInt(), 1);
    QCOMPARE(conn0.value("name").toString(), QString("Connection 1"));
    QCOMPARE(conn0.value("type").toString(), QString("tcp"));
    QCOMPARE(conn0.value("ip").toString(), QString("10.0.0.5"));
    QCOMPARE(conn0.value("port").toInt(), 502);
    QCOMPARE(conn0.value("portName").toString(), QString("COM1"));
    QCOMPARE(conn0.value("baudrate").toInt(), 115200);
    QCOMPARE(conn0.value("parity").toString(), QString("N"));
    QCOMPARE(conn0.value("databits").toInt(), 8);
    QCOMPARE(conn0.value("stopbits").toInt(), 1);
    QCOMPARE(conn0.value("timeout").toInt(), 1000);
    QVERIFY(conn0.value("persistent").toBool());

    const QJsonArray devices = adapterDevices(settings);
    QCOMPARE(devices.size(), 1);
    const QJsonObject dev0 = devices[0].toObject();
    QCOMPARE(dev0.value("id").toInt(), 1);
    QCOMPARE(dev0.value("connectionId").toInt(), 1);
    QCOMPARE(dev0.value("slaveId").toInt(), 1);
    QCOMPARE(dev0.value("consecutiveMax").toInt(), 125);
    QVERIFY(dev0.value("int32LittleEndian").toBool());
}

void TestProjectFileXmlParser::connectionNameFromId()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;
    QVERIFY(parser.parseFile(ProjectFileXmlTestData::cDataLevel5Demo, &settings).result());

    const QJsonArray connections = adapterConnections(settings);
    QCOMPARE(connections.size(), 2);
    QCOMPARE(connections[0].toObject().value("name").toString(), QString("Connection 1"));
    QCOMPARE(connections[1].toObject().value("name").toString(), QString("Connection 2"));
}

void TestProjectFileXmlParser::disabledUnusedConnectionDropped_data()
{
    QTest::addColumn<QString>("fileContent");

    QTest::newRow("datalevel 4 demo") << ProjectFileXmlTestData::cDataLevel4Demo;
    QTest::newRow("datalevel 5 demo") << ProjectFileXmlTestData::cDataLevel5Demo;
}

void TestProjectFileXmlParser::disabledUnusedConnectionDropped()
{
    QFETCH(QString, fileContent);

    ProjectFileXmlParser parser;
    ProjectSettings settings;
    QVERIFY(parser.parseFile(fileContent, &settings).result());

    /* Both demos hold connections 0 and 1 enabled and connection 2 disabled */
    QCOMPARE(connectionIds(adapterConnections(settings)), QList<int>({ 1, 2 }));
}

void TestProjectFileXmlParser::disabledReferencedConnectionKept()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;
    QVERIFY(parser.parseFile(ProjectFileXmlTestData::cDisabledReferencedConnection, &settings).result());

    const QJsonArray connections = adapterConnections(settings);
    QCOMPARE(connectionIds(connections), QList<int>({ 1, 2 }));
    QCOMPARE(connections[1].toObject().value("ip").toString(), QString("127.0.0.2"));
    QVERIFY(!connections[1].toObject().contains("enabled"));

    QCOMPARE(adapterDevices(settings)[0].toObject().value("connectionId").toInt(), 2);
}

void TestProjectFileXmlParser::duplicateConnectionIdsKeepFirst()
{
    const QString file = QString("<?xml version=\"1.0\"?>\n"
                                 "<modbusscope datalevel=\"5\">\n"
                                 " <modbus>\n"
                                 "  <connection>\n"
                                 "   <connectionid>1</connectionid>\n"
                                 "   <ip>10.0.0.1</ip>\n"
                                 "  </connection>\n"
                                 "  <connection>\n"
                                 "   <connectionid>1</connectionid>\n"
                                 "   <ip>10.0.0.2</ip>\n"
                                 "  </connection>\n"
                                 "  <device>\n"
                                 "   <deviceid>1</deviceid>\n"
                                 "   <connectionid>1</connectionid>\n"
                                 "  </device>\n"
                                 " </modbus>\n"
                                 "</modbusscope>\n");

    ProjectFileXmlParser parser;
    ProjectSettings settings;
    QVERIFY(parser.parseFile(file, &settings).result());

    /* XML connection id 1 is adapter connection id 2 */
    const QJsonArray connections = adapterConnections(settings);
    QCOMPARE(connections.size(), 1);
    QCOMPARE(connections[0].toObject().value("id").toInt(), 2);
    QCOMPARE(connections[0].toObject().value("ip").toString(), QString("10.0.0.1"));
}

void TestProjectFileXmlParser::deviceIdsUnchanged()
{
    ProjectFileXmlParser parser;

    /* Datalevel < 5: the device id is the 0-based connection id + 1, matching @N in expressions */
    ProjectSettings legacySettings;
    QVERIFY(parser.parseFile(ProjectFileXmlTestData::cDataLevel4Demo, &legacySettings).result());
    QCOMPARE(legacySettings.general.deviceSettings.size(), 2);
    QCOMPARE(legacySettings.general.deviceSettings[0].deviceId, static_cast<deviceId_t>(1));
    QCOMPARE(legacySettings.general.deviceSettings[1].deviceId, static_cast<deviceId_t>(2));
    const QJsonArray legacyDevices = adapterDevices(legacySettings);
    QCOMPARE(legacyDevices[0].toObject().value("id").toInt(), 1);
    QCOMPARE(legacyDevices[1].toObject().value("id").toInt(), 2);
    QCOMPARE(legacySettings.scope.registerList[1].expression, QString("${h1@2:s16b}"));

    /* Datalevel 5: the device id comes from <deviceid> */
    ProjectSettings settings;
    QVERIFY(parser.parseFile(ProjectFileXmlTestData::cDataLevel5MixedMulti, &settings).result());
    const QJsonArray devices = adapterDevices(settings);
    QCOMPARE(devices.size(), 3);
    QCOMPARE(devices[0].toObject().value("id").toInt(), 1);
    QCOMPARE(devices[1].toObject().value("id").toInt(), 2);
    QCOMPARE(devices[2].toObject().value("id").toInt(), 3);
}

void TestProjectFileXmlParser::demoDataLevel4ParsesCompletely()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;
    QVERIFY(parser.parseFile(ProjectFileXmlTestData::cDataLevel4Demo, &settings).result());

    QCOMPARE(settings.general.adapterList.size(), 1);
    QCOMPARE(settings.general.adapterList[0].type, QString("modbus"));

    const QJsonArray connections = adapterConnections(settings);
    QCOMPARE(connections.size(), 2);
    const QJsonObject conn0 = connections[0].toObject();
    QCOMPARE(conn0.value("id").toInt(), 1);
    QCOMPARE(conn0.value("type").toString(), QString("tcp"));
    QCOMPARE(conn0.value("ip").toString(), QString("127.0.0.1"));
    QCOMPARE(conn0.value("port").toInt(), 502);
    QCOMPARE(conn0.value("timeout").toInt(), 1000);
    const QJsonObject conn1 = connections[1].toObject();
    QCOMPARE(conn1.value("id").toInt(), 2);
    QCOMPARE(conn1.value("port").toInt(), 503);
    QCOMPARE(conn1.value("timeout").toInt(), 2000);

    const QJsonArray devices = adapterDevices(settings);
    QCOMPARE(devices.size(), 2);
    const QJsonObject dev0 = devices[0].toObject();
    QCOMPARE(dev0.value("id").toInt(), 1);
    QCOMPARE(dev0.value("connectionId").toInt(), 1);
    QCOMPARE(dev0.value("slaveId").toInt(), 1);
    QCOMPARE(dev0.value("consecutiveMax").toInt(), 100);
    QVERIFY(dev0.value("int32LittleEndian").toBool());
    const QJsonObject dev1 = devices[1].toObject();
    QCOMPARE(dev1.value("id").toInt(), 2);
    QCOMPARE(dev1.value("connectionId").toInt(), 2);
    QCOMPARE(dev1.value("slaveId").toInt(), 2);

    QCOMPARE(settings.scope.registerList.size(), 4);
    QVERIFY(settings.general.logSettings.bPollTime);
    QCOMPARE(settings.general.logSettings.pollTime, static_cast<quint32>(1000));
}

void TestProjectFileXmlParser::demoDataLevel5ParsesCompletely()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;
    QVERIFY(parser.parseFile(ProjectFileXmlTestData::cDataLevel5Demo, &settings).result());

    const QJsonArray connections = adapterConnections(settings);
    QCOMPARE(connections.size(), 2);
    QCOMPARE(connections[0].toObject().value("port").toInt(), 502);
    QCOMPARE(connections[1].toObject().value("port").toInt(), 503);

    const QJsonArray devices = adapterDevices(settings);
    QCOMPARE(devices.size(), 2);
    QCOMPARE(devices[0].toObject().value("id").toInt(), 1);
    QCOMPARE(devices[0].toObject().value("connectionId").toInt(), 1);
    QCOMPARE(devices[1].toObject().value("id").toInt(), 2);
    QCOMPARE(devices[1].toObject().value("connectionId").toInt(), 2);

    QCOMPARE(settings.general.deviceSettings.size(), 2);
    QCOMPARE(settings.general.deviceSettings[0].name, QString("Device 1"));
    QCOMPARE(settings.general.deviceSettings[1].name, QString("Device 2"));
    QCOMPARE(settings.scope.registerList.size(), 4);
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
    QVERIFY(qFuzzyIsNull(settings.view.scaleSettings.yAxis.scaleMin));
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
    QVERIFY(qFuzzyIsNull(settings.view.scaleSettings.y2Axis.scaleMin));
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

void TestProjectFileXmlParser::valueAxisInvalid()
{
    ProjectFileXmlParser parser;
    ProjectSettings settings;

    GeneralError err = parser.parseFile(ProjectFileXmlTestData::cValueAxisInvalid, &settings);
    QVERIFY(!err.result());
}

void TestProjectFileXmlParser::logFileRelativePath()
{
    /* Create a temporary project directory that contains "subdir/" so the relative path is valid */
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkdir("subdir"));

    ProjectFileXmlParser parserWithBase;
    ProjectSettings settingsWithBase;
    GeneralError errWithBase =
      parserWithBase.parseFile(ProjectFileXmlTestData::cLogFileRelativePath, &settingsWithBase, tempDir.path());
    QVERIFY(errWithBase.result());
    QVERIFY(settingsWithBase.general.logSettings.bLogToFileFile);
    QVERIFY(!settingsWithBase.general.logSettings.logFile.isEmpty());

    /* A different base dir without "subdir/" must reject the relative path */
    QTemporaryDir wrongDir;
    QVERIFY(wrongDir.isValid());

    ProjectFileXmlParser parserWrongBase;
    ProjectSettings settingsWrongBase;
    GeneralError errWrongBase =
      parserWrongBase.parseFile(ProjectFileXmlTestData::cLogFileRelativePath, &settingsWrongBase, wrongDir.path());
    QVERIFY(errWrongBase.result());
    QVERIFY(!settingsWrongBase.general.logSettings.bLogToFileFile);
}

QTEST_MAIN(TestProjectFileXmlParser)
