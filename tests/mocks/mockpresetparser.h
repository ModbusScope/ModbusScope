#ifndef MOCKPRESETPARSER_H__
#define MOCKPRESETPARSER_H__

#include "importexport/presetparser.h"

#include "gmock/gmock.h"

class MockPresetParser : public PresetParser {
    public:

        MOCK_METHOD(void, parsePresets, (QString fileContent), (override));
        MOCK_METHOD(PresetParser::Preset, preset, (quint32 index), (override));
        MOCK_METHOD(quint32, presetCount, (), (override));
};

#endif /* MOCKPRESETPARSER_H__ */
