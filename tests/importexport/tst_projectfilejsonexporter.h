
#ifndef TST_PROJECTFILEJSONEXPORTER_H
#define TST_PROJECTFILEJSONEXPORTER_H

#include <QObject>

class TestProjectFileJsonExporter : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void logObject();
    void logToFileWithPath();

    void scopeArray();

    void viewScaleSliding();
    void viewScaleMinMax();
    void viewScaleWindowAuto();
    void viewScaleAuto();

    void roundTrip();
};

#endif // TST_PROJECTFILEJSONEXPORTER_H
