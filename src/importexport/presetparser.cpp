
#include "scopelogging.h"
#include "presetparser.h"

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
    int size = _presetList.size();

    if (size < 0)
    {
        size = 0;
    }

    return static_cast<quint32>(size);
}

void PresetParser::parsePresets(QString fileContent)
{
    QString errorStr;
    qint32 errorLine;
    qint32 errorColumn;
    QDomDocument domDocument;

    _presetList.clear();

    if (!domDocument.setContent(fileContent, true, &errorStr, &errorLine, &errorColumn))
    {
        auto msg = QString(tr("Parse error at line %1, column %2:\n%3")
                .arg(errorLine)
                .arg(errorColumn)
                .arg(errorStr));
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
                    else
                    {
                        break;
                    }
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


bool PresetParser::parsePresetTag(const QDomElement &element, Preset *pPreset)
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
            if (child.text().trimmed().compare(QString("\\t"), Qt::CaseInsensitive) == 0)
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
            if (
                (!bRet)
                || (pPreset->labelRow < -1)
               )
            {
                qCWarning(scopePreset) << tr("Label row ( %1 ) is not a valid number. Specify -1 to indicate the absence of label row").arg(child.text());
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
        else if (child.tagName() == "dynamicsession")
        {
            if (!child.text().toLower().compare("true"))
            {
                pPreset->bDynamicSession = true;
            }
            else
            {
                pPreset->bDynamicSession = false;
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
        else if (child.tagName() == "stmstudiocorrection")
        {
            if (!child.text().toLower().compare("true"))
            {
                pPreset->bStmStudioCorrection = true;
            }
            else
            {
                pPreset->bStmStudioCorrection = false;
            }
        }
        else if (child.tagName() == "keyword")
        {
            pPreset->keyword = child.text();
        }
        else
        {
            // unkown tag: ignore
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
