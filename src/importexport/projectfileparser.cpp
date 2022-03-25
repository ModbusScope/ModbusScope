
#include <QFileInfo>
#include <QDir>
#include "util.h"
#include "projectfileparser.h"
#include "projectfiledefinitions.h"

using ProjectFileData::ProjectSettings;
using ProjectFileData::ConnectionSettings;
using ProjectFileData::LogSettings;
using ProjectFileData::ScopeSettings;
using ProjectFileData::RegisterSettings;
using ProjectFileData::ViewSettings;
using ProjectFileData::ScaleSettings;
using ProjectFileData::GeneralSettings;


ProjectFileParser::ProjectFileParser()
{

}


bool ProjectFileParser::parseFile(QString& fileContent, ProjectSettings *pSettings)
{
    bool bRet = true;
    QString errorStr;
    qint32 errorLine;
    qint32 errorColumn;

    if (!_domDocument.setContent(fileContent, true, &errorStr, &errorLine, &errorColumn))
    {
       Util::showError(tr("Parse error at line %1, column %2:\n%3")
                .arg(errorLine)
                .arg(errorColumn)
                .arg(errorStr));
        bRet = false;
    }
    else
    {
        QDomElement root = _domDocument.documentElement();
        if (root.tagName() != ProjectFileDefinitions::cModbusScopeTag)
        {
            Util::showError(tr("The file is not a valid ModbusScope project file."));
            bRet = false;
        }
        else
        {
            // Check data level attribute
            QString strDataLevel = root.attribute(ProjectFileDefinitions::cDatalevelAttribute, "1");
            quint32 datalevel = strDataLevel.toUInt(&bRet);

            if (bRet)
            {
                pSettings->dataLevel = datalevel;
                if (datalevel > ProjectFileDefinitions::cCurrentDataLevel)
                {
                    Util::showError(tr("Data level (%1) is not supported. Only datalevel %2 or lower is supported.\nProject file loading is aborted.")
                                       .arg(datalevel,ProjectFileDefinitions::cCurrentDataLevel));
                    bRet = false;
                }
            }
            else
            {
                Util::showError(tr("Data level (%1) is not a valid number").arg(strDataLevel));
            }

            if (bRet)
            {
                QDomElement tag = root.firstChildElement();
                while (!tag.isNull())
                {
                    if (tag.tagName() == ProjectFileDefinitions::cModbusTag)
                    {
                        bRet = parseModbusTag(tag, &pSettings->general);
                        if (!bRet)
                        {
                            break;
                        }
                    }
                    else if (tag.tagName() == ProjectFileDefinitions::cScopeTag)
                    {
                        bRet = parseScopeTag(tag, &pSettings->scope);
                        if (!bRet)
                        {
                            break;
                        }
                    }
                    else if (tag.tagName() == ProjectFileDefinitions::cViewTag)
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
    }

    return bRet;
}


bool ProjectFileParser::parseModbusTag(const QDomElement &element, GeneralSettings * pGeneralSettings)
{
    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cConnectionTag)
        {
            pGeneralSettings->connectionSettings.append(ConnectionSettings());

            bRet = parseConnectionTag(child, &pGeneralSettings->connectionSettings.last());
            if (!bRet)
            {
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cLogTag)
        {
            bRet = parseLogTag(child, &pGeneralSettings->logSettings);
            if (!bRet)
            {
                break;
            }
        }
        else
        {
            // unknown tag: ignore
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
        if (child.tagName() == ProjectFileDefinitions::cConnectionIdTag)
        {
            pConnectionSettings->bConnectionId = true;
            pConnectionSettings->connectionId = static_cast<quint8>(child.text().toUInt(&bRet));
            if (!bRet)
            {
                Util::showError(tr("Connection Id (%1) is not a valid number").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cConnectionEnabledTag)
        {
            if (!child.text().toLower().compare(ProjectFileDefinitions::cTrueValue))
            {
                pConnectionSettings->bConnectionState = true;
            }
            else
            {
                pConnectionSettings->bConnectionState = false;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cConnectionTypeTag)
        {
            pConnectionSettings->bConnectionType = true;
            pConnectionSettings->connectionType = child.text();
        }
        else if (child.tagName() == ProjectFileDefinitions::cIpTag)
        {
            pConnectionSettings->bIp = true;
            pConnectionSettings->ip = child.text();
        }
        else if (child.tagName() == ProjectFileDefinitions::cPortTag)
        {
            pConnectionSettings->bPort = true;
            pConnectionSettings->port = static_cast<quint16>(child.text().toUInt(&bRet));
            if (!bRet)
            {
                Util::showError(tr("Port ( %1 ) is not a valid number").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cPortNameTag)
        {
            pConnectionSettings->bPortName = true;
            pConnectionSettings->portName = child.text();
        }
        else if (child.tagName() == ProjectFileDefinitions::cBaudrateTag)
        {
            pConnectionSettings->bBaudrate = true;
            pConnectionSettings->baudrate = child.text().toUInt(&bRet);
            if (!bRet)
            {
                Util::showError(tr("Baud rate ( %1 ) is not a valid number").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cParityTag)
        {
            pConnectionSettings->bParity = true;
            pConnectionSettings->parity = child.text().toUInt(&bRet);
            if (!bRet)
            {
                Util::showError(tr("Parity ( %1 ) is not a valid number").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cStopBitsTag)
        {
            pConnectionSettings->bStopbits = true;
            pConnectionSettings->stopbits = child.text().toUInt(&bRet);
            if (!bRet)
            {
                Util::showError(tr("Stop bits ( %1 ) is not a valid number").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cDataBitsTag)
        {
            pConnectionSettings->bDatabits = true;
            pConnectionSettings->databits = child.text().toUInt(&bRet);
            if (!bRet)
            {
                Util::showError(tr("Data bits ( %1 ) is not a valid number").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cSlaveIdTag)
        {
            pConnectionSettings->bSlaveId = true;
            pConnectionSettings->slaveId = static_cast<quint8>(child.text().toUInt(&bRet));
            if (!bRet)
            {
                Util::showError(tr("Slave id ( %1 ) is not a valid number").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cTimeoutTag)
        {
            pConnectionSettings->bTimeout = true;
            pConnectionSettings->timeout = child.text().toUInt(&bRet);
            if (!bRet)
            {
                Util::showError(tr("Timeout ( %1 ) is not a valid number").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cConsecutiveMaxTag)
        {
            pConnectionSettings->bConsecutiveMax = true;
            pConnectionSettings->consecutiveMax = static_cast<quint8>(child.text().toUInt(&bRet));
            if (!bRet)
            {
                Util::showError(tr("Consecutive register maximum ( %1 ) is not a valid number").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cInt32LittleEndianTag)
        {
            if (!child.text().toLower().compare(ProjectFileDefinitions::cTrueValue))
            {
                pConnectionSettings->bInt32LittleEndian = true;
            }
            else
            {
                pConnectionSettings->bInt32LittleEndian = false;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cPersistentConnectionTag)
        {
            if (!child.text().toLower().compare(ProjectFileDefinitions::cTrueValue))
            {
                pConnectionSettings->bPersistentConnection = true;
            }
            else
            {
                pConnectionSettings->bPersistentConnection = false;
            }
        }
        else
        {
            // unknown tag: ignore
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
        if (child.tagName() == ProjectFileDefinitions::cPollTimeTag)
        {
            pLogSettings->bPollTime = true;
            pLogSettings->pollTime = child.text().toUInt(&bRet);
            if (!bRet)
            {
                Util::showError(tr("Poll time ( %1 ) is not a valid number").arg(child.text()));
                break;
            }
        } 
        else if (child.tagName() == ProjectFileDefinitions::cAbsoluteTimesTag)
        {
            if (!child.text().toLower().compare(ProjectFileDefinitions::cTrueValue))
            {
                pLogSettings->bAbsoluteTimes = true;
            }
            else
            {
                pLogSettings->bAbsoluteTimes = false;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cLogToFileTag)
        {
            bRet = parseLogToFile(child, pLogSettings);
            if (!bRet)
            {
                break;
            }
        }
        else
        {
            // unknown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    return bRet;
}

bool ProjectFileParser::parseLogToFile(const QDomElement &element, LogSettings *pLogSettings)
{
    bool bRet = true;

    // Check attribute
    QString enabled = element.attribute(ProjectFileDefinitions::cEnabledAttribute, ProjectFileDefinitions::cTrueValue);

    if (!enabled.compare(ProjectFileDefinitions::cTrueValue, Qt::CaseInsensitive))
    {
        pLogSettings->bLogToFile = true;
    }
    else
    {
        pLogSettings->bLogToFile = false;
    }

    // Check nodes
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cFilenameTag)
        {
            QFileInfo fileInfo = QFileInfo(child.text());

            bool bValid = true;

            /* check file path points to existing file */
            if (!fileInfo.isFile())
            {
                /* Check if file path points to something else that already exists */
                if (fileInfo.exists())
                {
                    /* path exist, but it is not a file */
                    bValid = false;
                    Util::showError(tr("Log file path (%1) already exists, but it is not a file. Log file is set to default.").arg(fileInfo.filePath()));
                }
                else
                {
                    /* file path does not exist yet */

                    /* Does parent directory exist? */
                    if (!fileInfo.dir().exists())
                    {
                        bValid = false;
                        Util::showError(tr("Log file path (parent directory) does not exists (%1). Log file is set to default.").arg(fileInfo.filePath()));
                    }
                }
            }

            if (bValid)
            {
                pLogSettings->bLogToFileFile = true;
                pLogSettings->logFile = fileInfo.filePath();
            }
            else
            {
                pLogSettings->bLogToFileFile = false;
            }
        }
        else
        {
            // unknown tag: ignore
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
        if (child.tagName() == ProjectFileDefinitions::cRegisterTag)
        {
            RegisterSettings registerData;
            bRet = parseRegisterTag(child, &registerData);
            if (!bRet)
            {
                break;
            }

            pScopeSettings->registerList.append(registerData);
        }
        else
        {
            // unknown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    return bRet;
}


bool ProjectFileParser::parseRegisterTag(const QDomElement &element, RegisterSettings *pRegisterSettings)
{
    bool bRet = true;

    // Check attribute
    QString active = element.attribute(ProjectFileDefinitions::cActiveAttribute, ProjectFileDefinitions::cTrueValue);

    if (!active.compare(ProjectFileDefinitions::cTrueValue, Qt::CaseInsensitive))
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
        if (child.tagName() == ProjectFileDefinitions::cAddressTag)
        {
            pRegisterSettings->address = static_cast<quint32>(child.text().toUInt(&bRet));
            if (!bRet)
            {
                Util::showError(tr("Address ( %1 ) is not a valid number").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cTextTag)
        {
            pRegisterSettings->text = child.text();
        }
        else if (child.tagName() == ProjectFileDefinitions::cUnsignedTag)
        {
            if (!child.text().toLower().compare(ProjectFileDefinitions::cTrueValue))
            {
                pRegisterSettings->bUnsigned = true;
            }
            else
            {
                pRegisterSettings->bUnsigned = false;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cBit32Tag)
        {
            if (!child.text().toLower().compare(ProjectFileDefinitions::cTrueValue))
            {
                pRegisterSettings->b32Bit = true;
            }
            else
            {
                pRegisterSettings->b32Bit = false;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cDivideTag)
        {
            // use settings from system locale
            pRegisterSettings->divideFactor = QLocale().toDouble(child.text(), &bRet);

            if (!bRet)
            {
                Util::showError(QString("Divide factor (%1) is not a valid double. Expected decimal separator is \"%2\".").arg(child.text()).arg(QLocale().decimalPoint()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cMultiplyTag)
        {
            // use settings from system locale
            pRegisterSettings->multiplyFactor = QLocale().toDouble(child.text(), &bRet);

            if (!bRet)
            {
                Util::showError(QString("Multiply factor (%1) is not a valid double. Expected decimal separator is \"%2\".").arg(child.text()).arg(QLocale().decimalPoint()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cColorTag)
        {
            bRet = QColor::isValidColor(child.text());
            pRegisterSettings->bColor = bRet;
            if (bRet)
            {
                pRegisterSettings->color = QColor(child.text());
            }
            else
            {
                Util::showError(tr("Color is not a valid color. Did you use correct color format? Expecting #FF0000 (red)"));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cBitmaskTag)
        {
            const quint32 newBitMask = static_cast<quint32>(child.text().toUInt(&bRet, 0));

            if (bRet)
            {
                pRegisterSettings->bitmask = newBitMask;
            }
            else
            {
                Util::showError(tr("Bitmask (\"%1\") is not a valid integer.").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cShiftTag)
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
                Util::showError(tr("Shift factor (%1) is not a valid integer between -16 and 16.").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cConnectionIdTag)
        {
            const qint32 newConnectionId = child.text().toInt(&bRet);

            if (
                    (bRet)
                    &&
                    (
                        (newConnectionId != 0)
                        &&
                        (newConnectionId != 1)
                    )
                )
            {
                bRet = false;
            }

            if (bRet)
            {
                pRegisterSettings->connectionId = static_cast<quint8>(newConnectionId);
            }
            else
            {
                Util::showError(tr("Connection id (%1) is not a valid integer between 0 and 1.").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cExpressionTag)
        {
            pRegisterSettings->bExpression = true;
            pRegisterSettings->expression = child.text();
        }
        else
        {
            // unknown tag: ignore
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
        if (child.tagName() == ProjectFileDefinitions::cScaleTag)
        {
            bRet = parseScaleTag(child, &pViewSettings->scaleSettings);
            if (!bRet)
            {
                break;
            }
        }
        else
        {
            // unknown tag: ignore
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
        if (child.tagName() == ProjectFileDefinitions::cXaxisTag)
        {
            // Check attribute
            QString active = child.attribute(ProjectFileDefinitions::cModeAttribute);

            if (!active.compare(ProjectFileDefinitions::cSlidingValue, Qt::CaseInsensitive))
            {
                // Sliding interval mode
                pScaleSettings->bSliding = true;

                bRet = parseScaleXAxis(child, pScaleSettings);
                if (!bRet)
                {
                    break;
                }
            }
            else if (!active.compare(ProjectFileDefinitions::cAutoValue, Qt::CaseInsensitive))
            {
                // auto interval mode
                pScaleSettings->bSliding = false;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cYaxisTag)
        {
            // Check attribute
            QString active = child.attribute(ProjectFileDefinitions::cModeAttribute);

            if (!active.compare(ProjectFileDefinitions::cWindowAutoValue, Qt::CaseInsensitive))
            {
                pScaleSettings->bWindowScale = true;
            }
            else if (!active.compare(ProjectFileDefinitions::cMinmaxValue, Qt::CaseInsensitive))
            {
                // min max mode
                pScaleSettings->bMinMax = true;

                bRet = parseScaleYAxis(child, pScaleSettings);
                if (!bRet)
                {
                    break;
                }
            }
            else if (!active.compare(ProjectFileDefinitions::cAutoValue, Qt::CaseInsensitive))
            {
                // auto interval mode
            }
        }
        else
        {
            // unknown tag: ignore
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
        if (child.tagName() == ProjectFileDefinitions::cSlidingintervalTag)
        {
            pScaleSettings->slidingInterval = child.text().toUInt(&bRet);
            if (bRet)
            {
                bSlidingInterval = true;
            }
            else
            {
                Util::showError(tr("Scale (x-axis) has an incorrect sliding interval. \"%1\" is not a valid number").arg(child.text()));
                break;
            }
        }
        else
        {
            // unknown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    if (!bSlidingInterval)
    {
        Util::showError(tr("If x-axis has sliding window scaling then slidinginterval variable should be defined."));
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
        if (child.tagName() == ProjectFileDefinitions::cMinTag)
        {
            pScaleSettings->scaleMin = QLocale().toDouble(child.text(), &bRet);
            if (bRet)
            {
                bMin = true;
            }
            else
            {
                Util::showError(tr("Scale (y-axis) has an incorrect minimum. \"%1\" is not a valid double").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cMaxTag)
        {
            pScaleSettings->scaleMax = QLocale().toDouble(child.text(), &bRet);
            if (bRet)
            {
                bMax = true;
            }
            else
            {
                Util::showError(tr("Scale (y-axis) has an incorrect maximum. \"%1\" is not a valid double").arg(child.text()));
                break;
            }
        }
        else
        {
            // unknown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    if (!bMin)
    {
        Util::showError(tr("If y-axis has min max scaling then min variable should be defined."));
        bRet = false;
    }
    else if (!bMax)
    {
        Util::showError(tr("If y-axis has min max scaling then max variable should be defined."));
        bRet = false;
    }

    return bRet;
}
