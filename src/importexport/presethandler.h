#ifndef PRESETHANDLER_H
#define PRESETHANDLER_H

#include <memory>

#include "presetparser.h"
#include <QObject>
#include <QStringList>

/* Forward declaration */
class DataParserModel;

class PresetHandler : public QObject
{
    Q_OBJECT
public:
    explicit PresetHandler(std::unique_ptr<PresetParser> pPresetParser, QObject* parent = nullptr);
    ~PresetHandler() = default;

    void loadPresetsFromFile(QString path);

    void fillWithPresetData(qint32 presetIndex, DataParserModel* pParserModel);
    QStringList nameList(void);
    qint32 determinePreset(QString filename);

    static void determinePresetFile(QString& presetFile);

signals:

private:
    std::unique_ptr<PresetParser> _pPresetParser;

    static constexpr QLatin1String _presetFilename{ "presets.xml" };
    static constexpr QLatin1String _presetFilenameJson{ "presets.json" };
};

#endif // PRESETHANDLER_H
