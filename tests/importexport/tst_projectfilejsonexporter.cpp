
#include "tst_projectfilejsonexporter.h"

#include "importexport/projectfilejsonexporter.h"
#include "importexport/projectfilejsonparser.h"
#include "models/graphdata.h"
#include "models/graphdatamodel.h"
#include "models/guimodel.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryFile>
#include <QTest>

using ProjectFileData::ProjectSettings;

namespace {

/*!
 * \brief Export models to a temp file and return the root JSON object.
 */
QJsonObject exportToJson(GuiModel& guiModel, SettingsModel& settingsModel, GraphDataModel& graphDataModel)
{
    QTemporaryFile tmpFile;
    QVERIFY(tmpFile.open());
    const QString path = tmpFile.fileName();
    tmpFile.close();

    ProjectFileJsonExporter exporter(&guiModel, &settingsModel, &graphDataModel);
    exporter.exportProjectFile(path);

    QFile file(path);
    QVERIFY(file.open(QFile::ReadOnly | QFile::Text));
    const QByteArray content = file.readAll();

    return QJsonDocument::fromJson(content).object();
}

} /* namespace */

void TestProjectFileJsonExporter::init()
{
}

void TestProjectFileJsonExporter::cleanup()
{
}

void TestProjectFileJsonExporter::logObject()
{
    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel;

    settingsModel.setPollTime(500);
    settingsModel.setAbsoluteTimes(true);
    settingsModel.setWriteDuringLog(false);

    const QJsonObject root = exportToJson(guiModel, settingsModel, graphDataModel);
    const QJsonObject logObj = root["log"].toObject();
    const QJsonObject logToFileObj = logObj["logtofile"].toObject();

    QCOMPARE(logObj["polltime"].toInt(), 500);
    QVERIFY(logObj["absolutetimes"].toBool());
    QVERIFY(!logToFileObj["enabled"].toBool());
}

void TestProjectFileJsonExporter::logToFileWithPath()
{
    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel;

    const QString logPath = QDir(QDir::tempPath()).filePath("tst_export_log.csv");
    settingsModel.setWriteDuringLog(true);
    settingsModel.setWriteDuringLogFile(logPath);

    const QJsonObject root = exportToJson(guiModel, settingsModel, graphDataModel);
    const QJsonObject logObj = root["log"].toObject();
    const QJsonObject logToFileObj = logObj["logtofile"].toObject();

    QVERIFY(logToFileObj["enabled"].toBool());
    QCOMPARE(logToFileObj["filename"].toString(), logPath);
}

void TestProjectFileJsonExporter::scopeArray()
{
    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel;

    GraphData reg1;
    reg1.setActive(true);
    reg1.setLabel(QString("Temperature"));
    reg1.setExpression(QString("${40001}"));
    reg1.setColor(QColor(0xff, 0x00, 0x00));
    reg1.setValueAxis(GraphData::VALUE_AXIS_PRIMARY);
    graphDataModel.add(reg1);

    GraphData reg2;
    reg2.setActive(false);
    reg2.setLabel(QString("Pressure"));
    reg2.setExpression(QString("${40002:s16b}/10"));
    reg2.setColor(QColor(0x00, 0x00, 0xff));
    reg2.setValueAxis(GraphData::VALUE_AXIS_SECONDARY);
    graphDataModel.add(reg2);

    const QJsonObject root = exportToJson(guiModel, settingsModel, graphDataModel);
    const QJsonArray scopeArray = root["scope"].toArray();

    QCOMPARE(scopeArray.size(), 2);

    const QJsonObject reg0 = scopeArray[0].toObject();
    QVERIFY(reg0["active"].toBool());
    QCOMPARE(reg0["text"].toString(), QString("Temperature"));
    QCOMPARE(reg0["expression"].toString(), QString("${40001}"));
    QCOMPARE(reg0["color"].toString(), QString("#ff0000"));
    QCOMPARE(reg0["valueaxis"].toInt(), static_cast<int>(GraphData::VALUE_AXIS_PRIMARY));

    const QJsonObject reg1Obj = scopeArray[1].toObject();
    QVERIFY(!reg1Obj["active"].toBool());
    QCOMPARE(reg1Obj["text"].toString(), QString("Pressure"));
    QCOMPARE(reg1Obj["expression"].toString(), QString("${40002:s16b}/10"));
    QCOMPARE(reg1Obj["color"].toString(), QString("#0000ff"));
    QCOMPARE(reg1Obj["valueaxis"].toInt(), static_cast<int>(GraphData::VALUE_AXIS_SECONDARY));
}

void TestProjectFileJsonExporter::viewScaleSliding()
{
    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel;

    guiModel.setxAxisScale(AxisMode::SCALE_SLIDING);
    guiModel.setxAxisSlidingInterval(60);

    const QJsonObject root = exportToJson(guiModel, settingsModel, graphDataModel);
    const QJsonObject viewObj = root["view"].toObject();
    const QJsonObject scaleObj = viewObj["scale"].toObject();
    const QJsonObject xAxis = scaleObj["xaxis"].toObject();

    QCOMPARE(xAxis["mode"].toString(), QString("sliding"));
    QCOMPARE(xAxis["slidinginterval"].toInt(), 60);
}

void TestProjectFileJsonExporter::viewScaleMinMax()
{
    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel;

    guiModel.setyAxisScale(AxisMode::SCALE_MINMAX);
    guiModel.setyAxisMin(-10.0);
    guiModel.setyAxisMax(200.5);

    const QJsonObject root = exportToJson(guiModel, settingsModel, graphDataModel);
    const QJsonObject viewObj = root["view"].toObject();
    const QJsonObject scaleObj = viewObj["scale"].toObject();
    const QJsonArray yAxes = scaleObj["yaxis"].toArray();
    const QJsonObject yAxis0 = yAxes[0].toObject();

    QCOMPARE(yAxis0["mode"].toString(), QString("minmax"));
    QCOMPARE(yAxis0["min"].toDouble(), -10.0);
    QCOMPARE(yAxis0["max"].toDouble(), 200.5);
}

void TestProjectFileJsonExporter::viewScaleWindowAuto()
{
    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel;

    guiModel.setyAxisScale(AxisMode::SCALE_WINDOW_AUTO);

    const QJsonObject root = exportToJson(guiModel, settingsModel, graphDataModel);
    const QJsonObject viewObj = root["view"].toObject();
    const QJsonObject scaleObj = viewObj["scale"].toObject();
    const QJsonArray yAxes = scaleObj["yaxis"].toArray();
    const QJsonObject yAxis0 = yAxes[0].toObject();

    QCOMPARE(yAxis0["mode"].toString(), QString("windowauto"));
}

void TestProjectFileJsonExporter::viewScaleAuto()
{
    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel;

    guiModel.setxAxisScale(AxisMode::SCALE_AUTO);
    guiModel.setyAxisScale(AxisMode::SCALE_AUTO);
    guiModel.sety2AxisScale(AxisMode::SCALE_AUTO);

    const QJsonObject root = exportToJson(guiModel, settingsModel, graphDataModel);
    const QJsonObject viewObj = root["view"].toObject();
    const QJsonObject scaleObj = viewObj["scale"].toObject();
    const QJsonObject xAxis = scaleObj["xaxis"].toObject();
    const QJsonArray yAxes = scaleObj["yaxis"].toArray();
    const QJsonObject yAxis0 = yAxes[0].toObject();
    const QJsonObject yAxis1 = yAxes[1].toObject();

    QCOMPARE(xAxis["mode"].toString(), QString("auto"));
    QCOMPARE(yAxis0["mode"].toString(), QString("auto"));
    QCOMPARE(yAxis1["mode"].toString(), QString("auto"));
}

void TestProjectFileJsonExporter::roundTrip()
{
    GuiModel guiModel;
    SettingsModel settingsModel;
    GraphDataModel graphDataModel;

    /* Configure models */
    settingsModel.setPollTime(250);
    settingsModel.setAbsoluteTimes(false);

    guiModel.setxAxisScale(AxisMode::SCALE_SLIDING);
    guiModel.setxAxisSlidingInterval(30);
    guiModel.setyAxisScale(AxisMode::SCALE_MINMAX);
    guiModel.setyAxisMin(-5.5);
    guiModel.setyAxisMax(100.25);
    guiModel.sety2AxisScale(AxisMode::SCALE_WINDOW_AUTO);

    GraphData reg;
    reg.setActive(true);
    reg.setLabel(QString("Channel 1"));
    reg.setExpression(QString("${40001}/2"));
    reg.setColor(QColor(0xff, 0x00, 0x00));
    reg.setValueAxis(GraphData::VALUE_AXIS_PRIMARY);
    graphDataModel.add(reg);

    /* Export */
    QTemporaryFile tmpFile;
    QVERIFY(tmpFile.open());
    const QString path = tmpFile.fileName();
    tmpFile.close();

    ProjectFileJsonExporter exporter(&guiModel, &settingsModel, &graphDataModel);
    exporter.exportProjectFile(path);

    /* Parse back */
    QFile file(path);
    QVERIFY(file.open(QFile::ReadOnly | QFile::Text));
    const QString json = QString::fromUtf8(file.readAll());

    ProjectFileJsonParser parser;
    ProjectSettings settings;
    const GeneralError err = parser.parseFile(json, &settings);

    QVERIFY(err.result());

    /* Verify round-trip fidelity for key fields */
    QVERIFY(settings.general.logSettings.bPollTime);
    QCOMPARE(settings.general.logSettings.pollTime, static_cast<quint32>(250));
    QVERIFY(!settings.general.logSettings.bAbsoluteTimes);

    QVERIFY(settings.view.scaleSettings.xAxis.bSliding);
    QCOMPARE(settings.view.scaleSettings.xAxis.slidingInterval, static_cast<quint32>(30));

    QVERIFY(settings.view.scaleSettings.yAxis.bMinMax);
    QCOMPARE(settings.view.scaleSettings.yAxis.scaleMin, -5.5);
    QCOMPARE(settings.view.scaleSettings.yAxis.scaleMax, 100.25);

    QVERIFY(settings.view.scaleSettings.y2Axis.bWindowScale);
    QVERIFY(!settings.view.scaleSettings.y2Axis.bMinMax);

    QCOMPARE(settings.scope.registerList.size(), 1);
    QVERIFY(settings.scope.registerList[0].bActive);
    QCOMPARE(settings.scope.registerList[0].text, QString("Channel 1"));
    QCOMPARE(settings.scope.registerList[0].expression, QString("${40001}/2"));
    QCOMPARE(settings.scope.registerList[0].color, QColor(0xff, 0x00, 0x00));
}

QTEST_MAIN(TestProjectFileJsonExporter)
