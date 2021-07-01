#ifndef PRESETHANDLER_H
#define PRESETHANDLER_H

#include <QObject>
#include <QStringList>
#include "presetparser.h"

/* Forward declaration */
class DataParserModel;

class PresetHandler : public QObject
{
    Q_OBJECT
public:
    explicit PresetHandler(PresetParser* pPresetParser, QObject *parent = nullptr);
    ~PresetHandler();

    void loadPresetsFromFile(QString path);

    void fillWithPresetData(qint32 presetIndex, DataParserModel* pParserModel);
    QStringList nameList(void);
    qint32 determinePreset(QString filename);

    static void determinePresetFile(QString &presetFile);

signals:

private:

    PresetParser* _pPresetParser;

    static const QString _presetFilename;

};

#endif // PRESETHANDLER_H
