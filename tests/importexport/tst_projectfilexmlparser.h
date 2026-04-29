
#ifndef TST_PROJECTFILEXMLPARSER_H
#define TST_PROJECTFILEXMLPARSER_H

#include <QObject>

class TestProjectFileXmlParser : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void initTestCase();
    void cleanup();

    void tooLowDataLevel();
    void tooHighDataLevel();

    void dataLevel3Expressions();

    void dataLevel5Connection();
    void dataLevel5MixedMulti();
    void dataLevel4LegacyConnection();

    void logSettings();

    void scaleSliding();
    void scaleMinMax();
    void scaleWindowAuto();

    void valueAxis();
    void valueAxisInvalid();

    void logFileRelativePath();
};

#endif // TST_PROJECTFILEXMLPARSER_H
