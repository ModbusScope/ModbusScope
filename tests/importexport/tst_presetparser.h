
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

private:
};

#endif // TST_PRESETPARSER_H
