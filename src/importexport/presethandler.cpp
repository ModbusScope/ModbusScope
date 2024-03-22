#include "presethandler.h"

#include <QStandardPaths>
#include <QFileInfo>
#include <QTextStream>

#include "dataparsermodel.h"
#include "scopelogging.h"

const QString PresetHandler::_presetFilename = QString("presets.xml");

PresetHandler::PresetHandler(PresetParser* pPresetParser, QObject *parent) : QObject(parent)
{
    _pPresetParser = pPresetParser;
}

PresetHandler::~PresetHandler()
{
    delete _pPresetParser;
}

QStringList PresetHandler::nameList(void)
{
    QStringList nameList;

    for (quint32 index = 0; index < _pPresetParser->presetCount(); index ++)
    {
        nameList.append(_pPresetParser->preset(index).name);
    }

    return nameList;
}

void PresetHandler::loadPresetsFromFile(QString path)
{
    if (!path.isEmpty())
    {
        QFile file(path);

        /* If we can't open it, let's show an error message. */
        if (file.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream in(&file);
            QString presetFileContent = in.readAll();

            _pPresetParser->parsePresets(presetFileContent);

            qCInfo(scopePreset) << QString("Preset file (%1) loaded: %2 preset(s) found").arg(path).arg(_pPresetParser->presetCount());
        }
        else
        {
            qCWarning(scopePreset) << tr("Couldn't open preset file: %1").arg(path);
        }
    }
}

void PresetHandler::fillWithPresetData(qint32 presetIndex, DataParserModel* pParserModel)
{
    if ((presetIndex >= 0) && (static_cast<quint32>(presetIndex) < _pPresetParser->presetCount()))
    {
        auto preset = _pPresetParser->preset(presetIndex);
        pParserModel->setColumn(preset.column -1);
        pParserModel->setDataRow(preset.dataRow - 1);
        pParserModel->setLabelRow(preset.labelRow - 1);
        pParserModel->setDecimalSeparator(preset.decimalSeparator);
        pParserModel->setFieldSeparator(preset.fieldSeparator);
        pParserModel->setGroupSeparator(preset.thousandSeparator);
        pParserModel->setCommentSequence(preset.commentSequence);
        pParserModel->setTimeInMilliSeconds(preset.bTimeInMilliSeconds);
    }
}

qint32 PresetHandler::determinePreset(QString filename)
{
    qint32 presetIdx = -1;

    // Loop through presets and set preset if keyword is in filename
    for (quint32 index = 0; index < _pPresetParser->presetCount(); index ++)
    {
        if (!_pPresetParser->preset(index).keyword.isEmpty())
        {
            if (QFileInfo(filename).fileName().contains(_pPresetParser->preset(index).keyword, Qt::CaseInsensitive))
            {
                presetIdx = static_cast<qint32>(index);
                break;
            }
        }
    }

    return presetIdx;
}

void PresetHandler::determinePresetFile(QString &presetFile)
{
    QString path;
    /* Check if preset file exists (2 locations)
    *   <document_folder>\ModbusScope\
    *   directory of executable
    */

    presetFile = "";

    QString documentsfolder;
    QStringList docPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    if (docPath.size() > 0)
    {
        documentsfolder = docPath[0];

        path = documentsfolder + "/ModbusScope/" + _presetFilename;
        if (QFileInfo::exists(path))
        {
            // xml in documents folder found
            presetFile = path;
        }
        else
        {
            // xml in documents folder doesn't exist, check directory of executable
            path = _presetFilename;
            if (QFileInfo::exists(path))
            {
                presetFile = path;
            }
        }
    }
}
