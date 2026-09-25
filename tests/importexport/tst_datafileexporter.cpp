#include "tst_datafileexporter.h"

#include "importexport/datafileexporter.h"
#include "importexport/datafileparser.h"
#include "models/communicationstatsmodel.h"
#include "models/dataparsermodel.h"
#include "models/graphdatamodel.h"
#include "models/notemodel.h"
#include "models/settingsmodel.h"
#include "util/util.h"

#include <QFile>
#include <QLocale>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTest>
#include <QTextStream>

using DataQuality::Flag;
using DataQuality::Quality;
using DataQuality::State;

namespace {

const QList<double> cTimestamps{ 0, 1000, 2000 };
const QList<double> cValuesTemp{ 21.5, 21.25, 0 };
const QList<double> cValuesPress{ 1.5, 0, 3 };
const QList<Quality> cQualitiesTemp{ Quality{ State::Good, Flag::NoFlags },
                                     Quality{ State::Degraded, Flag::OldData | Flag::Substituted },
                                     Quality{ State::Invalid, Flag::NoFlags } };
const QList<Quality> cQualitiesPress{ Quality{ State::Degraded, Flag::Overflow },
                                      Quality{ State::NoValue, Flag::NoFlags }, Quality{ State::Good, Flag::NoFlags } };

//! Fills \a graphDataModel with two signals that carry mixed data quality
void fillGraphDataModel(GraphDataModel& graphDataModel)
{
    graphDataModel.add(QStringList() << "Temp" << "Press");
    graphDataModel.mutableDataSeries(GraphIdx(0))->setSamples(cTimestamps, cValuesTemp, cQualitiesTemp);
    graphDataModel.mutableDataSeries(GraphIdx(1))->setSamples(cTimestamps, cValuesPress, cQualitiesPress);
}

//! Exports \a graphDataModel to \a path and returns the lines of the written file
QStringList exportToLines(const QString& path, GraphDataModel& graphDataModel)
{
    SettingsModel settingsModel;
    CommunicationStatsModel communicationStatsModel;
    NoteModel noteModel;
    DataFileExporter exporter(&settingsModel, &graphDataModel, &communicationStatsModel, &noteModel);

    exporter.exportDataFile(path);

    QStringList lines;
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        while (!stream.atEnd())
        {
            lines.append(stream.readLine());
        }
    }
    return lines;
}

} // namespace

void TestDataFileExporter::exportContainsQualityColumns()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    GraphDataModel graphDataModel;
    fillGraphDataModel(graphDataModel);

    const QStringList lines = exportToLines(dir.filePath("data.csv"), graphDataModel);
    const QString sep = Util::separatorCharacter();

    QVERIFY(lines.contains(QString("//Quality") + sep + "1"));

    const qsizetype labelIdx =
      lines.indexOf(QStringList{ "Time (ms)", "Temp", "Temp (quality)", "Press", "Press (quality)" }.join(sep));
    QVERIFY(labelIdx >= 0);
    QCOMPARE(lines.size(), labelIdx + 1 + cTimestamps.size());

    /* Degraded (1) with Substituted (16) and OldData (128) */
    QCOMPARE(lines[labelIdx + 2].split(sep)[2], QString("145"));
}

void TestDataFileExporter::exportRoundTripKeepsQuality()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("data.csv");

    GraphDataModel graphDataModel;
    fillGraphDataModel(graphDataModel);

    const QStringList lines = exportToLines(path, graphDataModel);
    const qsizetype labelIdx = lines.indexOf(QRegularExpression("^Time \\(ms\\).*"));
    QVERIFY(labelIdx >= 0);

    DataParserModel dataParserModel;
    dataParserModel.setFieldSeparator(Util::separatorCharacter());
    dataParserModel.setGroupSeparator(QChar(' '));
    dataParserModel.setDecimalSeparator(QLocale().decimalPoint());
    dataParserModel.setCommentSequence(QString("//"));
    dataParserModel.setLabelRow(static_cast<quint32>(labelIdx));
    dataParserModel.setDataRow(static_cast<quint32>(labelIdx + 1));
    dataParserModel.setColumn(static_cast<quint32>(0));
    dataParserModel.setTimeInMilliSeconds(true);

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream stream(&file);

    DataFileParser parser(&dataParserModel);
    DataFileParser::FileData fileData;
    QVERIFY(parser.processDataFile(&stream, &fileData));

    QCOMPARE(fileData.dataLabel, QStringList() << "Temp" << "Press");
    QCOMPARE(fileData.timeRow, cTimestamps);
    QCOMPARE(fileData.dataRows, QList<QList<double> >() << cValuesTemp << cValuesPress);
    QVERIFY(fileData.qualityRows == (QList<QList<Quality> >() << cQualitiesTemp << cQualitiesPress));
}

QTEST_GUILESS_MAIN(TestDataFileExporter)
