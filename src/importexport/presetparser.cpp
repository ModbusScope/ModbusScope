
#include "presetparser.h"

#include "importexport/legacy/presetxmlparser.h"
#include "util/scopelogging.h"

PresetParser::PresetParser() = default;

PresetParser::Preset PresetParser::preset(quint32 index)
{
    return _presetList.at(index);
}

quint32 PresetParser::presetCount()
{
    return static_cast<quint32>(_presetList.size());
}

void PresetParser::parsePresets(QString fileContent)
{
    _presetList.clear();

    if (fileContent.trimmed().startsWith('{'))
    {
        parsePresetsFromJson(fileContent);
    }
    else
    {
        PresetXmlParser xmlParser;
        xmlParser.parsePresets(fileContent, &_presetList);
    }
}

/*!
 * \brief Parses preset entries from a JSON string and appends valid presets to the internal list.
 * \param fileContent The full JSON file content. Expected to be a JSON object with a "presets" array.
 */
void PresetParser::parsePresetsFromJson(const QString& fileContent)
{
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(fileContent.toUtf8(), &jsonError);

    if (doc.isNull())
    {
        qCWarning(scopePreset) << tr("JSON parse error: %1").arg(jsonError.errorString());
        return;
    }

    if (!doc.isObject())
    {
        qCWarning(scopePreset) << tr("JSON preset file must be a JSON object at the root level.");
        return;
    }

    QJsonObject root = doc.object();

    if (!root.contains("presets") || !root.value("presets").isArray())
    {
        qCWarning(scopePreset) << tr("JSON preset file missing 'presets' array");
        return;
    }

    QJsonArray presetsArray = root.value("presets").toArray();

    for (const QJsonValue& value : std::as_const(presetsArray))
    {
        if (!value.isObject())
        {
            qCWarning(scopePreset) << tr("Each preset entry must be a JSON object.");
            continue;
        }

        Preset preset;
        if (parsePresetFromJson(value.toObject(), &preset))
        {
            _presetList.append(preset);
        }
        // else: invalid preset, skip and continue loading remaining presets
    }
}

bool PresetParser::parsePresetFromJson(const QJsonObject& obj, Preset* pPreset)
{
    if (!obj.contains("name") || !obj.value("name").isString())
    {
        qCWarning(scopePreset) << tr("Name is not specified.");
        return false;
    }
    pPreset->name = obj.value("name").toString();

    if (!obj.contains("fieldseparator") || !obj.value("fieldseparator").isString())
    {
        qCWarning(scopePreset) << tr("Field separator is not specified (%1).").arg(pPreset->name);
        return false;
    }
    QString fieldSepStr = obj.value("fieldseparator").toString();
    if (fieldSepStr.trimmed().compare(QString(R"(\t)"), Qt::CaseInsensitive) == 0)
    {
        pPreset->fieldSeparator = '\t';
    }
    else if (!fieldSepStr.isEmpty())
    {
        pPreset->fieldSeparator = fieldSepStr.at(0);
    }
    else
    {
        qCWarning(scopePreset) << tr("Field separator is empty");
        return false;
    }

    if (!obj.contains("decimalseparator") || !obj.value("decimalseparator").isString())
    {
        qCWarning(scopePreset) << tr("Decimal separator is not specified (%1).").arg(pPreset->name);
        return false;
    }
    QString decimalSepStr = obj.value("decimalseparator").toString();
    if (!decimalSepStr.isEmpty())
    {
        pPreset->decimalSeparator = decimalSepStr.at(0);
    }
    else
    {
        qCWarning(scopePreset) << tr("Decimal separator is empty");
        return false;
    }

    if (obj.contains("thousandseparator"))
    {
        QString thousandSepStr = obj.value("thousandseparator").toString();
        pPreset->thousandSeparator = !thousandSepStr.isEmpty() ? thousandSepStr.at(0) : QChar(' ');
    }

    if (obj.contains("commentsequence"))
    {
        pPreset->commentSequence = obj.value("commentsequence").toString();
    }

    if (obj.contains("column"))
    {
        bool ok = obj.value("column").isDouble() && (obj.value("column").toInt(-1) >= 0);
        if (!ok)
        {
            qCWarning(scopePreset) << tr("Column is not a valid number.");
            return false;
        }
        pPreset->column = static_cast<quint32>(obj.value("column").toInt());
    }

    if (obj.contains("labelrow"))
    {
        qint32 labelRow = obj.value("labelrow").toInt(-2);
        bool ok = obj.value("labelrow").isDouble() && (labelRow >= -1);
        if (!ok)
        {
            qCWarning(scopePreset) << tr(
              "Label row is not a valid number. Specify -1 to indicate the absence of label row");
            return false;
        }
        pPreset->labelRow = labelRow;
    }

    if (obj.contains("datarow"))
    {
        bool ok = obj.value("datarow").isDouble() && (obj.value("datarow").toInt(-1) >= 0);
        if (!ok)
        {
            qCWarning(scopePreset) << tr("Data row is not a valid number.");
            return false;
        }
        pPreset->dataRow = static_cast<quint32>(obj.value("datarow").toInt());
    }

    if (obj.contains("timeinmilliseconds"))
    {
        pPreset->bTimeInMilliSeconds = obj.value("timeinmilliseconds").toBool(true);
    }

    if (obj.contains("keyword"))
    {
        pPreset->keyword = obj.value("keyword").toString();
    }

    return true;
}
