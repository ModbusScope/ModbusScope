
#ifndef TST_PROJECTFILEJSONPARSER_H
#define TST_PROJECTFILEJSONPARSER_H

#include <QObject>

class TestProjectFileJsonParser : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void initTestCase();
    void cleanup();

    void tooLowVersion();
    void tooHighVersion();
    void missingVersion();

    void adaptersAndDevices();
    void multipleAdaptersAndDevices();
    void adapterOnlyNoDevices();

    void logSettings();
    void logToFileWithPath();

    void scopeRegisters();

    void viewScaleSliding();
    void viewScaleMinMax();
    void viewScaleWindowAuto();
    void viewScaleAuto();
};

#endif // TST_PROJECTFILEJSONPARSER_H
