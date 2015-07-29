
#include <QtWidgets>
#include "util.h"
#include "projectfileparser.h"

ProjectFileParser::ProjectFileParser()
{
    _msgBox.setWindowTitle(tr("ModbusScope project file load error"));
    _msgBox.setIcon(QMessageBox::Warning);
}


bool ProjectFileParser::parseFile(QIODevice *device, ProjectSettings *pSettings)
{
    bool bRet = true;
    QString errorStr;
    qint32 errorLine;
    qint32 errorColumn;

    if (!_domDocument.setContent(device, true, &errorStr, &errorLine, &errorColumn))
    {
        _msgBox.setText(tr("Parse error at line %1, column %2:\n%3")
                .arg(errorLine)
                .arg(errorColumn)
                .arg(errorStr));
        _msgBox.exec();

        bRet = false;
    }
    else
    {
        QDomElement root = _domDocument.documentElement();
        if (root.tagName() != "modbusscope")
        {
            _msgBox.setText(tr("The file is not a valid ModbusScope project file."));
            _msgBox.exec();
            bRet = false;
        }
        else
        {
            QDomElement tag = root.firstChildElement();
            while (!tag.isNull())
            {
                if (tag.tagName() == "modbus")
                {
                    bRet = parseModbusTag(tag, &pSettings->general);
                    if (!bRet)
                    {
                        break;
                    }
                }
                else if (tag.tagName() == "scope")
                {
                    bRet = parseScopeTag(tag, &pSettings->scope);
                    if (!bRet)
                    {
                        break;
                    }
                }
                else if (tag.tagName() == "scale")
                {
                    bRet = parseScaleTag(tag, &pSettings->scale);
                    if (!bRet)
                    {
                        break;
                    }
                }
                else if (tag.tagName() == "legend")
                {
                    bRet = parseLegendTag(tag, &pSettings->legend);
                    if (!bRet)
                    {
                        break;
                    }
                }
                tag = tag.nextSiblingElement();
            }
        }
    }

    return bRet;
}


bool ProjectFileParser::parseModbusTag(const QDomElement &element, GeneralSettings * pGeneralSettings)
{
    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "ip")
        {
            pGeneralSettings->bIp = true;
            pGeneralSettings->ip = child.text();
        }
        else if (child.tagName() == "port")
        {
            pGeneralSettings->bPort = true;
            pGeneralSettings->port = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Port ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else if (child.tagName() == "polltime")
        {
            pGeneralSettings->bPollTime = true;
            pGeneralSettings->pollTime = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Poll time ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else if (child.tagName() == "slaveid")
        {
            pGeneralSettings->bSlaveId = true;
            pGeneralSettings->slaveId = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Slave id ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();

                break;
            }
        }
        else if (child.tagName() == "timeout")
        {
            pGeneralSettings->bTimeout = true;
            pGeneralSettings->timeout = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Timeout ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();

                break;
            }
        }
        else if (child.tagName() == "consecutivemax")
        {
            pGeneralSettings->bConsecutiveMax = true;
            pGeneralSettings->consecutiveMax = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Consecutive register maximum ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();

                break;
            }
        }
        else
        {
            // unkown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    return bRet;
}


bool ProjectFileParser::parseScopeTag(const QDomElement &element, ScopeSettings *pScopeSettings)
{
    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "var")
        {
            RegisterSettings registerData;
            bRet = parseVariableTag(child, &registerData);
            if (!bRet)
            {
                break;
            }

            // check for duplicate registers
            bool bFound = false;

            for (int i = 0; i < pScopeSettings->registerList.size(); i++)
            {
                if (
                        (pScopeSettings->registerList[i].address == registerData.address)
                        && (pScopeSettings->registerList[i].bitmask == registerData.bitmask)
                    )
                {
                    bFound = true;
                    break;
                }
            }

            if (bFound)
            {
                _msgBox.setText(tr("Register %1 with bitmask 0x%2 is defined twice in the list.").arg(registerData.address).arg(registerData.bitmask, 0, 16));
                _msgBox.exec();
                bRet = false;
                break;
            }
            else
            {
                pScopeSettings->registerList.append(registerData);
            }
        }
        else
        {
            // unkown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    return bRet;
}


bool ProjectFileParser::parseVariableTag(const QDomElement &element, RegisterSettings *pRegisterSettings)
{
    bool bRet = true;

    // Check attribute
    QString active = element.attribute("active", "false");

    if (!active.toLower().compare("true"))
    {
        pRegisterSettings->bActive = true;
    }
    else
    {
        pRegisterSettings->bActive = false;
    }

    // Check nodes
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "reg")
        {
            pRegisterSettings->address = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Register ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else if (child.tagName() == "text")
        {
            pRegisterSettings->text = child.text();
        }
        else if (child.tagName() == "unsigned")
        {
            if (!child.text().toLower().compare("true"))
            {
                pRegisterSettings->bUnsigned = true;
            }
            else
            {
                pRegisterSettings->bUnsigned = false;
            }
        }
        else if (child.tagName() == "scalefactor")
        {
            pRegisterSettings->scaleFactor = QLocale::system().toDouble(child.text(), &bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Scale factor is not a valid double. Did you use correct decimal separator character? Expecting \"%1\"").arg(QLocale::system().decimalPoint()));
                _msgBox.exec();
                break;
            }
        }
        else if (child.tagName() == "color")
        {
            bRet = QColor::isValidColor(child.text());
            pRegisterSettings->bColor = bRet;
            if (bRet)
            {
                pRegisterSettings->color = QColor(child.text());
            }
            else
            {
                _msgBox.setText(tr("Color is not a valid double. Did you use correct color format? Expecting #FF0000 (red)"));
                _msgBox.exec();
                break;
            }
        }
        else if (child.tagName() == "bitmask")
        {
            const quint16 newBitMask = child.text().toUInt(&bRet, 0);

            if (bRet)
            {
                pRegisterSettings->bitmask = newBitMask;
            }
            else
            {
                _msgBox.setText(tr("Bitmask (\"%1\") is not a valid integer.").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else
        {
            // unkown tag: ignore
        }

        child = child.nextSiblingElement();
    }

    return bRet;
}


bool ProjectFileParser::parseScaleTag(const QDomElement &element, ScaleSettings *pScaleSettings)
{
    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "xaxis")
        {
            // Check attribute
            QString active = child.attribute("mode");

            if (!active.toLower().compare("sliding"))
            {
                // Sliding interval mode
                pScaleSettings->bSliding = true;

                bRet = parseScaleXAxis(child, pScaleSettings);
                if (!bRet)
                {
                    break;
                }
            }
            else if (!active.toLower().compare("auto"))
            {
                // auto interval mode
                pScaleSettings->bSliding = false;
            }
        }
        else if (child.tagName() == "yaxis")
        {
            // Check attribute
            QString active = child.attribute("mode");

            if (!active.toLower().compare("minmax"))
            {
                // min max mode
                pScaleSettings->bMinMax = true;

                bRet = parseScaleYAxis(child, pScaleSettings);
                if (!bRet)
                {
                    break;
                }
            }
            else if (!active.toLower().compare("auto"))
            {
                // auto interval mode
                pScaleSettings->bMinMax = false;
            }
        }
        else
        {
            // unkown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    return bRet;
}

bool ProjectFileParser::parseScaleXAxis(const QDomElement &element, ScaleSettings *pScaleSettings)
{
    bool bRet = true;
    bool bSlidingInterval = false;

    // Check nodes
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "slidinginterval")
        {
            pScaleSettings->slidingInterval = child.text().toUInt(&bRet);
            if (bRet)
            {
                bSlidingInterval = true;
            }
            else
            {
                _msgBox.setText(tr("Scale (x-axis) has an incorrect sliding interval. \"%1\" is not a valid number").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else
        {
            // unkown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    if (!bSlidingInterval)
    {
        _msgBox.setText(tr("If x-axis has sliding window scaling then slidinginterval variable should be defined."));
        _msgBox.exec();
        bRet = false;
    }

    return bRet;
}

bool ProjectFileParser::parseScaleYAxis(const QDomElement &element, ScaleSettings *pScaleSettings)
{
    bool bRet = true;
    bool bMin = false;
    bool bMax = false;

    // Check nodes
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "min")
        {
            pScaleSettings->scaleMin = child.text().toInt(&bRet);
            if (bRet)
            {
                bMin = true;
            }
            else
            {
                _msgBox.setText(tr("Scale (y-axis) has an incorrect minimum. \"%1\" is not a valid number").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else if (child.tagName() == "max")
        {
            pScaleSettings->scaleMax = child.text().toInt(&bRet);
            if (bRet)
            {
                bMax = true;
            }
            else
            {
                _msgBox.setText(tr("Scale (y-axis) has an incorrect maximum. \"%1\" is not a valid number").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else
        {
            // unkown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    if (!bMin)
    {
        _msgBox.setText(tr("If y-axis has min max scaling then min variable should be defined."));
        _msgBox.exec();
        bRet = false;
    }
    else if (!bMax)
    {
        _msgBox.setText(tr("If y-axis has min max scaling then max variable should be defined."));
        _msgBox.exec();
        bRet = false;
    }

    return bRet;
}

bool ProjectFileParser::parseLegendTag(const QDomElement &element, LegendSettings *pLegendSettings)
{
    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "position")
        {
            if (child.text().trimmed().toLower() == "left")
            {
                pLegendSettings->bLegendPosition = true;
                pLegendSettings->legendPosition = 0;
            }
            else if (child.text().trimmed().toLower() == "middle")
            {
                pLegendSettings->bLegendPosition = true;
                pLegendSettings->legendPosition = 1;
            }
            else if (child.text().trimmed().toLower() == "right")
            {
                pLegendSettings->bLegendPosition = true;
                pLegendSettings->legendPosition = 2;
            }
            else
            {
                _msgBox.setText(tr("Legend position has an incorrect value. Only left, middle and right values are allowed"));
                _msgBox.exec();
                break;
            }
        }
        else
        {
            // unkown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    return bRet;
}
