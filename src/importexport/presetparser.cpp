
#include "presetparser.h"

#include "util/scopelogging.h"

PresetParser::PresetParser()
{
    _presetList.clear();
}

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
        QDomDocument domDocument;

        QDomDocument::ParseResult result =
          domDocument.setContent(fileContent, QDomDocument::ParseOption::UseNamespaceProcessing);
        if (!result)
        {
            auto msg = QString(tr("Parse error at line %1, column %2:\n%3")
                                 .arg(result.errorLine)
                                 .arg(result.errorColumn)
                                 .arg(result.errorMessage));
            qCWarning(scopePreset) << msg;
        }
        else
        {
            QDomElement root = domDocument.documentElement();
            if (root.tagName() == "modbusscope")
            {
                QDomElement tag = root.firstChildElement();
                while (!tag.isNull())
                {
                    if (tag.tagName() == "parsepreset")
                    {
                        Preset preset;
                        if (parsePresetTag(tag, &preset))
                        {
                            _presetList.append(preset);
                        }
                        // else: invalid preset, skip and continue loading remaining presets
                    }
                    else
                    {
                        // unknown: ignore
                    }
                    tag = tag.nextSiblingElement();
                }
            }
        }
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

    if (!doc.object().contains("presets") || !doc.object().value("presets").isArray())
    {
        qCWarning(scopePreset) << tr("JSON preset file missing 'presets' array");
        return;
    }

    QJsonArray presetsArray = doc.object().value("presets").toArray();

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

bool PresetParser::parsePresetTag(const QDomElement& element, Preset* pPreset)
{
    bool bFieldseparator = false;
    bool bName = false;
    bool bDecimalSeparator = false;

    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "name")
        {
            bName = true;
            pPreset->name = child.text();
        }
        else if (child.tagName() == "fieldseparator")
        {
            bFieldseparator = true;
            if (child.text().trimmed().compare(QString(R"(\t)"), Qt::CaseInsensitive) == 0)
            {
                pPreset->fieldSeparator = '\t';
            }
            else if (child.text().size() > 0)
            {
                pPreset->fieldSeparator = child.text().at(0);
            }
            else
            {
                // No data
                bRet = false;
                qCWarning(scopePreset) << tr("Field separator is empty");
                break;
            }
        }
        else if (child.tagName() == "decimalseparator")
        {
            bDecimalSeparator = true;
            if (child.text().size() > 0)
            {
                pPreset->decimalSeparator = child.text().at(0);
            }
            else
            {
                bRet = false;
                qCWarning(scopePreset) << tr("Decimal separator is empty");
                break;
            }
        }
        else if (child.tagName() == "thousandseparator")
        {
            if (child.text().size() > 0)
            {
                pPreset->thousandSeparator = child.text().at(0);
            }
            else
            {
                pPreset->thousandSeparator = ' ';
            }
        }
        else if (child.tagName() == "commentSequence")
        {
            pPreset->commentSequence = child.text();
        }
        else if (child.tagName() == "column")
        {
            pPreset->column = child.text().toUInt(&bRet);
            if (!bRet)
            {
                qCWarning(scopePreset) << tr("Column ( %1 ) is not a valid number.").arg(child.text());
                break;
            }
        }
        else if (child.tagName() == "labelrow")
        {
            pPreset->labelRow = child.text().toInt(&bRet);
            if ((!bRet) || (pPreset->labelRow < -1))
            {
                qCWarning(scopePreset)
                  << tr("Label row ( %1 ) is not a valid number. Specify -1 to indicate the absence of label row")
                       .arg(child.text());
                break;
            }
        }
        else if (child.tagName() == "datarow")
        {
            pPreset->dataRow = child.text().toUInt(&bRet);
            if (!bRet)
            {
                qCWarning(scopePreset) << tr("Data row ( %1 ) is not a valid number.").arg(child.text());
                break;
            }
        }
        else if (child.tagName() == "timeinmilliseconds")
        {
            if (!child.text().toLower().compare("true"))
            {
                pPreset->bTimeInMilliSeconds = true;
            }
            else
            {
                pPreset->bTimeInMilliSeconds = false;
            }
        }
        else if (child.tagName() == "keyword")
        {
            pPreset->keyword = child.text();
        }
        else
        {
            // unknown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    if (!bName)
    {
        qCWarning(scopePreset) << tr("Name is not specified.");
        bRet = false;
    }
    else if (!bFieldseparator)
    {
        qCWarning(scopePreset) << tr("Field separator is not specified (%1).").arg(pPreset->name);
        bRet = false;
    }
    else if (!bDecimalSeparator)
    {
        qCWarning(scopePreset) << tr("Decimal separator is not specified (%1).").arg(pPreset->name);
        bRet = false;
    }

    return bRet;
}
