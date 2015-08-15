
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
                else if (tag.tagName() == "view")
                {
                    bRet = parseViewTag(tag, &pSettings->view);
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
        if (child.tagName() == "connection")
        {
            bRet = parseConnectionTag(child, &pGeneralSettings->connectionSettings);
            if (!bRet)
            {
                break;
            }
        }
        else if (child.tagName() == "log")
        {
            bRet = parseLogTag(child, &pGeneralSettings->logSettings);
            if (!bRet)
            {
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

bool ProjectFileParser::parseConnectionTag(const QDomElement &element, ConnectionSettings * pConnectionSettings)
{
    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "ip")
        {
            pConnectionSettings->bIp = true;
            pConnectionSettings->ip = child.text();
        }
        else if (child.tagName() == "port")
        {
            pConnectionSettings->bPort = true;
            pConnectionSettings->port = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Port ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else if (child.tagName() == "slaveid")
        {
            pConnectionSettings->bSlaveId = true;
            pConnectionSettings->slaveId = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Slave id ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();

                break;
            }
        }
        else if (child.tagName() == "timeout")
        {
            pConnectionSettings->bTimeout = true;
            pConnectionSettings->timeout = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Timeout ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();

                break;
            }
        }
        else if (child.tagName() == "consecutivemax")
        {
            pConnectionSettings->bConsecutiveMax = true;
            pConnectionSettings->consecutiveMax = child.text().toUInt(&bRet);
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

bool ProjectFileParser::parseLogTag(const QDomElement &element, LogSettings * pLogSettings)
{
    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
         if (child.tagName() == "polltime")
        {
            pLogSettings->bPollTime = true;
            pLogSettings->pollTime = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Poll time ( %1 ) is not a valid number").arg(child.text()));
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
        if (child.tagName() == "register")
        {
            RegisterSettings registerData;
            bRet = parseRegisterTag(child, &registerData);
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


bool ProjectFileParser::parseRegisterTag(const QDomElement &element, RegisterSettings *pRegisterSettings)
{
    bool bRet = true;

    // Check attribute
    QString active = element.attribute("active", "true");

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
        if (child.tagName() == "address")
        {
            pRegisterSettings->address = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Address ( %1 ) is not a valid number").arg(child.text()));
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
        else if (child.tagName() == "divide")
        {
            // set to C locale => '.' as decimal separator
            QLocale locale = QLocale("C");

            pRegisterSettings->divideFactor = locale.toDouble(child.text(), &bRet);

            if (!bRet)
            {
                _msgBox.setText(tr("Divide factor (%1) is not a valid double. Expected decimal separator is point").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else if (child.tagName() == "multiply")
        {
            // set to C locale => '.' as decimal separator
            QLocale locale = QLocale("C");

            pRegisterSettings->multiplyFactor = locale.toDouble(child.text(), &bRet);

            if (!bRet)
            {
                _msgBox.setText(tr("Multiply factor (%1) is not a valid double. Expected decimal separator is point").arg(child.text()));
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
                _msgBox.setText(tr("Color is not a valid color. Did you use correct color format? Expecting #FF0000 (red)"));
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
        else if (child.tagName() == "shift")
        {
            const qint32 newShift = child.text().toInt(&bRet);

            if (
                    (bRet)
                    &&
                    (
                        (newShift < -15)
                        ||
                        (newShift > 15)
                    )
                )
            {
                bRet = false;
            }

            if (bRet)
            {
                pRegisterSettings->shift = newShift;
            }
            else
            {
                _msgBox.setText(tr("Shift factor (%1) is not a valid integer between -16 and 16.").arg(child.text()));
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

bool ProjectFileParser::parseViewTag(const QDomElement &element, ViewSettings *pViewSettings)
{
    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "scale")
        {
            bRet = parseScaleTag(child, &pViewSettings->scaleSettings);
            if (!bRet)
            {
                break;
            }
        }
        else if (child.tagName() == "legend")
        {
            bRet = parseLegendTag(child, &pViewSettings->legendSettings);
            if (!bRet)
            {
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

    // Check attribute
    QString visibility = element.attribute("visible", "true");

    if (!visibility.toLower().compare("true"))
    {
        pLegendSettings->bVisible = true;
    }
    else
    {
        pLegendSettings->bVisible = false;
    }

    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "position")
        {
            if (child.text().trimmed().toLower() == "left")
            {
                pLegendSettings->bPosition = true;
                pLegendSettings->position = 0;
            }
            else if (child.text().trimmed().toLower() == "middle")
            {
                pLegendSettings->bPosition = true;
                pLegendSettings->position = 1;
            }
            else if (child.text().trimmed().toLower() == "right")
            {
                pLegendSettings->bPosition = true;
                pLegendSettings->position = 2;
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
