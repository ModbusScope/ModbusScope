#ifndef TST_DATAFILEEXPORTER_H
#define TST_DATAFILEEXPORTER_H

#include <QObject>

class TestDataFileExporter : public QObject
{
    Q_OBJECT

private slots:
    void exportContainsQualityColumns();
    void exportRoundTripKeepsQuality();
};

#endif // TST_DATAFILEEXPORTER_H
