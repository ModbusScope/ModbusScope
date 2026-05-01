
#include "presetxmlparser.h"

#include "util/scopelogging.h"

#include <QDomDocument>
#include <QDomElement>

/*!
 * \brief Parses preset entries from a legacy XML string and appends valid presets to the list.
 * \param fileContent The full XML file content.
 * \param pPresetList List to append successfully parsed presets to.
 */
void PresetXmlParser::parsePresets(const QString& fileContent, QList<PresetParser::Preset>* pPresetList)
{
    QDomDocument domDocument;

    QDomDocument::ParseResult result =
      domDocument.setContent(fileContent, QDomDocument::ParseOption::UseNamespaceProcessing);
    if (!result)
    {
        auto msg = QString("Parse error at line %1, column %2:\n%3")
                     .arg(result.errorLine)
                     .arg(result.errorColumn)
                     .arg(result.errorMessage);
        qCWarning(scopePreset) << qUtf8Printable(msg);
        return;
    }

    QDomElement root = domDocument.documentElement();
    if (root.tagName() != "modbusscope")
    {
        qCWarning(scopePreset) << QString("Unexpected root tag '%1', expected 'modbusscope'").arg(root.tagName());
        return;
    }

    QDomElement tag = root.firstChildElement();
    while (!tag.isNull())
    {
        if (tag.tagName() == "parsepreset")
        {
            PresetParser::Preset preset;
            if (parsePresetTag(tag, &preset))
            {
                pPresetList->append(preset);
            }
            // else: invalid preset, skip and continue loading remaining presets
        }
        // else: unknown tag, ignore
        tag = tag.nextSiblingElement();
    }
}

bool PresetXmlParser::parsePresetTag(const QDomElement& element, PresetParser::Preset* pPreset)
{
    bool bFieldseparator = false;
    bool bName = false;
    bool bDecimalSeparator = false;

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
                qCWarning(scopePreset) << "Field separator is empty";
                return false;
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
                qCWarning(scopePreset) << "Decimal separator is empty";
                return false;
            }
        }
        else if (child.tagName() == "thousandseparator")
        {
            pPreset->thousandSeparator = (child.text().size() > 0) ? child.text().at(0) : QChar(' ');
        }
        else if (child.tagName() == "commentSequence") // camelCase kept intentionally: matches legacy XML files
        {
            pPreset->commentSequence = child.text();
        }
        else if (child.tagName() == "column")
        {
            bool ok = false;
            pPreset->column = child.text().toUInt(&ok);
            if (!ok)
            {
                qCWarning(scopePreset) << QString("Column ( %1 ) is not a valid number.").arg(child.text());
                return false;
            }
        }
        else if (child.tagName() == "labelrow")
        {
            bool ok = false;
            pPreset->labelRow = child.text().toInt(&ok);
            if (!ok || pPreset->labelRow < -1)
            {
                qCWarning(scopePreset)
                  << QString("Label row ( %1 ) is not a valid number. Specify -1 to indicate the absence of label row")
                       .arg(child.text());
                return false;
            }
        }
        else if (child.tagName() == "datarow")
        {
            bool ok = false;
            pPreset->dataRow = child.text().toUInt(&ok);
            if (!ok)
            {
                qCWarning(scopePreset) << QString("Data row ( %1 ) is not a valid number.").arg(child.text());
                return false;
            }
        }
        else if (child.tagName() == "timeinmilliseconds")
        {
            pPreset->bTimeInMilliSeconds = (child.text().toLower() == "true");
        }
        else if (child.tagName() == "keyword")
        {
            pPreset->keyword = child.text();
        }
        // else: unknown tag, ignore

        child = child.nextSiblingElement();
    }

    if (!bName)
    {
        qCWarning(scopePreset) << "Name is not specified.";
        return false;
    }
    if (!bFieldseparator)
    {
        qCWarning(scopePreset) << QString("Field separator is not specified (%1).").arg(pPreset->name);
        return false;
    }
    if (!bDecimalSeparator)
    {
        qCWarning(scopePreset) << QString("Decimal separator is not specified (%1).").arg(pPreset->name);
        return false;
    }

    return true;
}
