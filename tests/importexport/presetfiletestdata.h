
#ifndef PRESETFILETESTDATA_H
#define PRESETFILETESTDATA_H

#include <QObject>

class PresetFileTestData : public QObject
{
    Q_OBJECT

public:
    static QString cSinglePreset;
    static QString cInvalidFile;
    static QString cSinglePresetJson;
    static QString cInvalidFileJson;
    static QString cMixedInvalidEntries;

    static QString cXmlTabSeparator;
    static QString cXmlSkipInvalidLoadValid;
    static QString cXmlOptionalFields;
    static QString cXmlMinimalPreset;
    static QString cJsonTabSeparator;
    static QString cJsonOptionalFields;
    static QString cJsonMinimalPreset;

private:
};

#endif // PRESETFILETESTDATA_H
