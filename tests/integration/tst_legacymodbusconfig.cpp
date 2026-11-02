#include "tst_legacymodbusconfig.h"

#include "../importexport/legacy/projectfilexmltestdata.h"
#include "ProtocolAdapter/adapterclient.h"
#include "ProtocolAdapter/adapterprocess.h"
#include "importexport/legacy/projectfilexmlparser.h"
#include "models/adapterdata.h"
#include "models/settingsmodel.h"

#include <QFileInfo>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

#include <memory>

namespace {

constexpr int cSessionTimeoutMs = 10000;
constexpr const char* const cAdapterId = "modbus";

} // namespace

void TestLegacyModbusConfig::initTestCase()
{
    const QString executable = QString::fromUtf8(MODBUS_ADAPTER_EXECUTABLE);
    QVERIFY2(QFileInfo::exists(executable),
             qPrintable(QStringLiteral("MODBUS_ADAPTER_EXECUTABLE not found on disk: %1").arg(executable)));
    QVERIFY2(QFileInfo(executable).isExecutable(),
             qPrintable(QStringLiteral("MODBUS_ADAPTER_EXECUTABLE is not executable: %1").arg(executable)));
}

void TestLegacyModbusConfig::init()
{
    _pClient = new AdapterClient(std::make_unique<AdapterProcess>(), QString(), this);
}

void TestLegacyModbusConfig::cleanup()
{
    delete _pClient;
    _pClient = nullptr;
}

void TestLegacyModbusConfig::legacyProjectConfiguresAdapter_data()
{
    QTest::addColumn<QString>("fileContent");

    QTest::newRow("datalevel 3 connections") << ProjectFileXmlTestData::cDataLevel3Connections;
    QTest::newRow("datalevel 4 demo") << ProjectFileXmlTestData::cDataLevel4Demo;
    QTest::newRow("datalevel 4 legacy connection") << ProjectFileXmlTestData::cDataLevel4LegacyConnection;
    QTest::newRow("datalevel 4 without connection ids") << ProjectFileXmlTestData::cDataLevel4LegacyConnectionNoIds;
    QTest::newRow("datalevel 5 demo") << ProjectFileXmlTestData::cDataLevel5Demo;
    QTest::newRow("datalevel 5 connection") << ProjectFileXmlTestData::cDataLevel5Connection;
    QTest::newRow("datalevel 5 mixed") << ProjectFileXmlTestData::cDataLevel5MixedMulti;
    QTest::newRow("serial variants") << ProjectFileXmlTestData::cSerialVariants;
    QTest::newRow("out of range values") << ProjectFileXmlTestData::cOutOfRangeValues;
    QTest::newRow("minimal connection") << ProjectFileXmlTestData::cMinimalConnection;
    QTest::newRow("disabled referenced connection") << ProjectFileXmlTestData::cDisabledReferencedConnection;
}

/*!
 * \brief The config built from a legacy XML project is accepted by the real adapter.
 *
 * A rejected adapter.configure does not fail the session: AdapterClient degrades it and reports
 * the rejection as an "error" diagnostic, so the test checks the diagnostics as well.
 */
void TestLegacyModbusConfig::legacyProjectConfiguresAdapter()
{
    QFETCH(QString, fileContent);

    ProjectFileXmlParser parser;
    ProjectFileData::ProjectSettings settings;
    QVERIFY(parser.parseFile(fileContent, &settings).result());
    QCOMPARE(settings.general.adapterList.size(), 1);

    SettingsModel settingsModel;
    QSignalSpy spyDescribe(_pClient, &AdapterClient::describeResult);
    QSignalSpy spyStarted(_pClient, &AdapterClient::sessionStarted);
    QSignalSpy spyError(_pClient, &AdapterClient::sessionError);
    QSignalSpy spyDiagnostic(_pClient, &AdapterClient::diagnosticReceived);

    _pClient->prepareAdapter(QString::fromUtf8(MODBUS_ADAPTER_EXECUTABLE));
    QVERIFY2(spyDescribe.wait(cSessionTimeoutMs), "No describeResult signal received");

    const QString adapterId = QString::fromUtf8(cAdapterId);
    settingsModel.updateAdapterFromDescribe(adapterId, spyDescribe.at(0).at(0).value<QJsonObject>());
    settingsModel.setAdapterCurrentConfig(adapterId, settings.general.adapterList[0].settings);

    _pClient->provideConfig(settingsModel.adapterData(adapterId)->configForWire(), QStringList());

    QVERIFY2(spyStarted.wait(cSessionTimeoutMs), "sessionStarted not emitted");
    QCOMPARE(spyError.count(), 0);

    for (const QList<QVariant>& diagnostic : std::as_const(spyDiagnostic))
    {
        QVERIFY2(diagnostic.at(0).toString() != QStringLiteral("error"), qPrintable(diagnostic.at(1).toString()));
    }
}

QTEST_GUILESS_MAIN(TestLegacyModbusConfig)
