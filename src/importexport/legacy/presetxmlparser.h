#ifndef PRESETXMLPARSER_H
#define PRESETXMLPARSER_H

#include "importexport/presetparser.h"

#include <QList>
#include <QString>

class QDomElement;

class PresetXmlParser
{
public:
    void parsePresets(const QString& fileContent, QList<PresetParser::Preset>* pPresetList);

private:
    bool parsePresetTag(const QDomElement& element, PresetParser::Preset* pPreset);
};

#endif // PRESETXMLPARSER_H
