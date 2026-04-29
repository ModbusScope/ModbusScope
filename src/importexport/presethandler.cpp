#include "presethandler.h"

#include "models/dataparsermodel.h"
#include "util/scopelogging.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTextStream>

const QString PresetHandler::_presetFilename = QString("presets.xml");
const QString PresetHandler::_presetFilenameJson = QString("presets.json");

/*!
 * \brief Constructs a PresetHandler.
 * \param pPresetParser Ownership-transferred parser used to read preset files.
 * \param parent Optional parent QObject.
 */
PresetHandler::PresetHandler(std::unique_ptr<PresetParser> pPresetParser, QObject* parent)
    : QObject(parent), _pPresetParser(std::move(pPresetParser))
{
}

/*!
 * \brief Returns the list of preset names known to the parser.
 * \return QStringList with one entry per loaded preset, in load order.
 */
QStringList PresetHandler::nameList(void)
{
    QStringList nameList;

    for (quint32 index = 0; index < _pPresetParser->presetCount(); index++)
    {
        nameList.append(_pPresetParser->preset(index).name);
    }

    return nameList;
}

/*!
 * \brief Reads and parses a preset file from the given path.
 * \param path Absolute path to the preset file (JSON or XML). Does nothing if empty.
 */
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

            qCInfo(scopePreset)
              << QString("Preset file (%1) loaded: %2 preset(s) found").arg(path).arg(_pPresetParser->presetCount());
        }
        else
        {
            qCWarning(scopePreset) << tr("Couldn't open preset file: %1").arg(path);
        }
    }
}

/*!
 * \brief Applies the preset at \a presetIndex to \a pParserModel.
 * \param presetIndex Zero-based index into the loaded preset list. Out-of-range values are ignored.
 * \param pParserModel Model to populate with the preset's field/decimal/label settings.
 */
void PresetHandler::fillWithPresetData(qint32 presetIndex, DataParserModel* pParserModel)
{
    if ((presetIndex >= 0) && (static_cast<quint32>(presetIndex) < _pPresetParser->presetCount()))
    {
        auto preset = _pPresetParser->preset(presetIndex);
        pParserModel->setColumn(preset.column - 1);
        pParserModel->setDataRow(preset.dataRow - 1);
        pParserModel->setLabelRow(preset.labelRow - 1);
        pParserModel->setDecimalSeparator(preset.decimalSeparator);
        pParserModel->setFieldSeparator(preset.fieldSeparator);
        pParserModel->setGroupSeparator(preset.thousandSeparator);
        pParserModel->setCommentSequence(preset.commentSequence);
        pParserModel->setTimeInMilliSeconds(preset.bTimeInMilliSeconds);
    }
}

/*!
 * \brief Finds the first preset whose keyword matches \a filename.
 * \param filename File name (or full path) to match against preset keywords.
 * \return Zero-based preset index, or -1 if no keyword matches.
 */
qint32 PresetHandler::determinePreset(QString filename)
{
    qint32 presetIdx = -1;

    // Loop through presets and set preset if keyword is in filename
    for (quint32 index = 0; index < _pPresetParser->presetCount(); index++)
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

/*!
 * \brief Resolves the path of the preset file to load, preferring JSON over XML.
 * \param presetFile Set to the resolved absolute path, or an empty string if no preset file is found.
 */
void PresetHandler::determinePresetFile(QString& presetFile)
{
    /* Check if preset file exists (2 locations, JSON preferred over XML in each)
     *   <document_folder>\ModbusScope\
     *   directory of executable
     */

    presetFile = "";

    QStringList docPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    if (!docPath.isEmpty())
    {
        QString documentsfolder = docPath[0];
        QString basePath = documentsfolder + "/ModbusScope/";

        if (QFileInfo::exists(basePath + _presetFilenameJson))
        {
            presetFile = basePath + _presetFilenameJson;
            return;
        }

        if (QFileInfo::exists(basePath + _presetFilename))
        {
            presetFile = basePath + _presetFilename;
            return;
        }
    }

    QString appDir = QCoreApplication::applicationDirPath();

    if (QFileInfo::exists(appDir + "/" + _presetFilenameJson))
    {
        presetFile = appDir + "/" + _presetFilenameJson;
        return;
    }

    if (QFileInfo::exists(appDir + "/" + _presetFilename))
    {
        presetFile = appDir + "/" + _presetFilename;
    }
}
