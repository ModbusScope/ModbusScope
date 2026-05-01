
#ifndef TST_PRESETPARSER_H
#define TST_PRESETPARSER_H

#include <QObject>

class TestPresetFileParser : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void singlePreset();
    void invalidFile();
    void singlePresetJson();
    void invalidFileJson();
    void skipInvalidThenLoadValid();

    void tabSeparator();
    void tabSeparatorJson();
    void xmlSkipInvalidThenLoadValid();
    void optionalFieldsXml();
    void optionalFieldsJson();
    void minimalPresetXml();
    void minimalPresetJson();

private:
};

#endif // TST_PRESETPARSER_H
