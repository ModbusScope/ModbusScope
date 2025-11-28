
#include "projectfileparser.h"

#include "importexport/projectfiledefinitions.h"

#include <QDir>
#include <QFileInfo>

using ProjectFileData::ConnectionSettings;
using ProjectFileData::DeviceSettings;
using ProjectFileData::GeneralSettings;
using ProjectFileData::LogSettings;
using ProjectFileData::ProjectSettings;
using ProjectFileData::RegisterSettings;
using ProjectFileData::ScaleSettings;
using ProjectFileData::ScopeSettings;
using ProjectFileData::ViewSettings;
using ProjectFileData::YAxisSettings;

ProjectFileParser::ProjectFileParser()
{

}

GeneralError ProjectFileParser::parseFile(QString& fileContent, ProjectSettings *pSettings)
{
    GeneralError parseErr;

    QDomDocument::ParseResult result =
      _domDocument.setContent(fileContent, QDomDocument::ParseOption::UseNamespaceProcessing);
    if (!result)
    {
        parseErr.reportError(QString("Parse error at line %1, column %2:\n%3")
                               .arg(result.errorLine)
                               .arg(result.errorColumn)
                               .arg(result.errorMessage));
    }
    else
    {
        QDomElement root = _domDocument.documentElement();
        if (root.tagName() != ProjectFileDefinitions::cModbusScopeTag)
        {
            parseErr.reportError(QString("The file is not a valid ModbusScope project file."));
        }
        else
        {
            bool bRet;
            // Check data level attribute
            QString strDataLevel = root.attribute(ProjectFileDefinitions::cDatalevelAttribute, "1");
            quint32 datalevel = strDataLevel.toUInt(&bRet);

            if (bRet)
            {
                _dataLevel = datalevel;
                if (datalevel < ProjectFileDefinitions::cMinimumDataLevel)
                {
                    parseErr.reportError(QString("Data level %1 is not supported. Minimum datalevel is %2.\n\n"
                                       "Try saving the project file with a previous version of ModbusScope.\n\n"
                                       "Project file loading is aborted.")
                                        .arg(datalevel)
                                        .arg(ProjectFileDefinitions::cMinimumDataLevel));
                }
                else if (datalevel > ProjectFileDefinitions::cCurrentDataLevel)
                {
                    parseErr.reportError(QString("Data level %1 is not supported. Only datalevel %2 or lower is supported.\n\n"
                                       "Try upgrading ModbusScope to the latest version.\n\n"
                                       "Project file loading is aborted.")
                                        .arg(datalevel)
                                        .arg(ProjectFileDefinitions::cCurrentDataLevel));
                }
                else
                {
                    /* Supported datalevel */
                }
            }
            else
            {
                parseErr.reportError(QString("Data level (%1) is not a valid number").arg(strDataLevel));
            }

            if (parseErr.result())
            {
                QDomElement tag = root.firstChildElement();
                while (!tag.isNull())
                {
                    if (tag.tagName() == ProjectFileDefinitions::cModbusTag)
                    {
                        parseErr = parseModbusTag(tag, &pSettings->general);
                        if (!parseErr.result())
                        {
                            break;
                        }
                    }
                    else if (tag.tagName() == ProjectFileDefinitions::cScopeTag)
                    {
                        parseErr = parseScopeTag(tag, &pSettings->scope);
                        if (!parseErr.result())
                        {
                            break;
                        }
                    }
                    else if (tag.tagName() == ProjectFileDefinitions::cViewTag)
                    {
                        parseErr = parseViewTag(tag, &pSettings->view);
                        if (!parseErr.result())
                        {
                            break;
                        }
                    }

                    tag = tag.nextSiblingElement();
                }
            }
        }
    }

    return parseErr;
}


GeneralError ProjectFileParser::parseModbusTag(const QDomElement &element, GeneralSettings * pGeneralSettings)
{
    GeneralError parseErr;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cConnectionTag)
        {
            pGeneralSettings->connectionSettings.append(ConnectionSettings());

            if (_dataLevel < 5)
            {
                DeviceSettings deviceSettings;
                parseErr =
                  parseLegacyConnectionTag(child, &pGeneralSettings->connectionSettings.last(), &deviceSettings);

                if (parseErr.result() && pGeneralSettings->connectionSettings.last().bConnectionState)
                {
                    deviceSettings.bConnectionId = true;
                    deviceSettings.connectionId = pGeneralSettings->connectionSettings.last().connectionId;
                    pGeneralSettings->deviceSettings.append(deviceSettings);
                }
            }
            else
            {
                parseErr = parseConnectionTag(child, &pGeneralSettings->connectionSettings.last());
            }

            if (!parseErr.result())
            {
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cDeviceTag)
        {
            pGeneralSettings->deviceSettings.append(DeviceSettings());

            parseErr = parseDeviceTag(child, &pGeneralSettings->deviceSettings.last());
            if (!parseErr.result())
            {
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cLogTag)
        {
            parseErr = parseLogTag(child, &pGeneralSettings->logSettings);
            if (!parseErr.result())
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

    return parseErr;
}

GeneralError ProjectFileParser::parseConnectionTag(const QDomElement &element, ConnectionSettings * pConnectionSettings)
{
    GeneralError parseErr;
    QDomElement child = element.firstChildElement();
    while (!child.isNull() && parseErr.result())
    {
        parseErr = parseConnectionFields(child, pConnectionSettings);

        child = child.nextSiblingElement();
    }

    return parseErr;
}

GeneralError ProjectFileParser::parseLegacyConnectionTag(const QDomElement& element,
                                                         ConnectionSettings* pConnectionSettings,
                                                         DeviceSettings* pDeviceSettings)
{
    GeneralError parseErr;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        bool bRet;

        parseErr = parseConnectionFields(child, pConnectionSettings);

        if (!parseErr.result())
        {
            break;
        }
        else if (child.tagName() == ProjectFileDefinitions::cSlaveIdTag)
        {
            pDeviceSettings->bSlaveId = true;
            pDeviceSettings->slaveId = static_cast<quint8>(child.text().toUInt(&bRet));
            if (!bRet)
            {
                parseErr.reportError(QString("Slave id ( %1 ) is not a valid number").arg(child.text()));
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cConsecutiveMaxTag)
        {
            pDeviceSettings->bConsecutiveMax = true;
            pDeviceSettings->consecutiveMax = static_cast<quint8>(child.text().toUInt(&bRet));
            if (!bRet)
            {
                parseErr.reportError(
                  QString("Consecutive register maximum ( %1 ) is not a valid number").arg(child.text()));
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cInt32LittleEndianTag)
        {
            if (!child.text().toLower().compare(ProjectFileDefinitions::cTrueValue))
            {
                pDeviceSettings->bInt32LittleEndian = true;
            }
            else
            {
                pDeviceSettings->bInt32LittleEndian = false;
            }
        }
        else
        {
            // unknown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    return parseErr;
}

GeneralError ProjectFileParser::parseDeviceTag(const QDomElement& element, DeviceSettings* pDeviceSettings)
{
    GeneralError parseErr;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        bool bRet;
        if (child.tagName() == ProjectFileDefinitions::cDeviceIdTag)
        {
            pDeviceSettings->bDeviceId = true;
            pDeviceSettings->deviceId = child.text().toUInt(&bRet);
            if (!bRet)
            {
                parseErr.reportError(QString("Device Id (%1) is not a valid number").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cDeviceNameTag)
        {
            pDeviceSettings->bName = true;
            pDeviceSettings->name = child.text();
        }
        else if (child.tagName() == ProjectFileDefinitions::cConnectionIdTag)
        {
            pDeviceSettings->bConnectionId = true;
            pDeviceSettings->connectionId = child.text().toUInt(&bRet);
            if (!bRet)
            {
                parseErr.reportError(QString("Connection Id (%1) is not a valid number").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cSlaveIdTag)
        {
            pDeviceSettings->bSlaveId = true;
            pDeviceSettings->slaveId = static_cast<quint8>(child.text().toUInt(&bRet));
            if (!bRet)
            {
                parseErr.reportError(QString("Slave id ( %1 ) is not a valid number").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cConsecutiveMaxTag)
        {
            pDeviceSettings->bConsecutiveMax = true;
            pDeviceSettings->consecutiveMax = static_cast<quint8>(child.text().toUInt(&bRet));
            if (!bRet)
            {
                parseErr.reportError(
                  QString("Consecutive register maximum ( %1 ) is not a valid number").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cInt32LittleEndianTag)
        {
            if (!child.text().toLower().compare(ProjectFileDefinitions::cTrueValue))
            {
                pDeviceSettings->bInt32LittleEndian = true;
            }
            else
            {
                pDeviceSettings->bInt32LittleEndian = false;
            }
        }
        else
        {
            // unknown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    return parseErr;
}

GeneralError ProjectFileParser::parseLogTag(const QDomElement &element, LogSettings * pLogSettings)
{
    GeneralError parseErr;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cPollTimeTag)
        {
            bool bRet;
            pLogSettings->bPollTime = true;
            pLogSettings->pollTime = child.text().toUInt(&bRet);
            if (!bRet)
            {
                parseErr.reportError(QString("Poll time ( %1 ) is not a valid number").arg(child.text()));
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
            parseErr = parseLogToFile(child, pLogSettings);
            if (!parseErr.result())
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

    return parseErr;
}

GeneralError ProjectFileParser::parseLogToFile(const QDomElement &element, LogSettings *pLogSettings)
{
    GeneralError parseErr;

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
                    parseErr.reportError(QString("Log file path (%1) already exists, but it is not a file. Log file is set to default.").arg(fileInfo.filePath()));
                }
                else
                {
                    /* file path does not exist yet */

                    /* Does parent directory exist? */
                    if (!fileInfo.dir().exists())
                    {
                        bValid = false;
                        parseErr.reportError(QString("Log file path (parent directory) does not exists (%1). Log file is set to default.").arg(fileInfo.filePath()));
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

    return parseErr;
}

GeneralError ProjectFileParser::parseScopeTag(const QDomElement &element, ScopeSettings *pScopeSettings)
{
    GeneralError parseErr;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cRegisterTag)
        {
            RegisterSettings registerData;
            parseErr = parseRegisterTag(child, &registerData);
            if (!parseErr.result())
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

    return parseErr;
}


GeneralError ProjectFileParser::parseRegisterTag(const QDomElement &element, RegisterSettings *pRegisterSettings)
{
    GeneralError parseErr;

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
        bool bRet;
        if (child.tagName() == ProjectFileDefinitions::cTextTag)
        {
            pRegisterSettings->text = child.text();
        }
        else if (child.tagName() == ProjectFileDefinitions::cColorTag)
        {
            bRet = QColor::isValidColorName(child.text());
            pRegisterSettings->bColor = bRet;
            if (bRet)
            {
                pRegisterSettings->color = QColor(child.text());
            }
            else
            {
                parseErr.reportError(QString("Color is not a valid color. Did you use correct color format? Expecting #FF0000 (red)"));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cValueAxisTag)
        {
            bool bOk = false;
            quint32 axis = child.text().toUInt(&bOk);

            if (bOk)
            {
                pRegisterSettings->valueAxis = axis;
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
                pRegisterSettings->connectionId = static_cast<ConnectionTypes::connectionId_t>(newConnectionId);
            }
            else
            {
                parseErr.reportError(QString("Connection id (%1) is not a valid integer between 0 and 1.").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cExpressionTag)
        {
            pRegisterSettings->expression = child.text();
        }
        else
        {
            // unknown tag: ignore
        }

        child = child.nextSiblingElement();
    }

    return parseErr;
}

GeneralError ProjectFileParser::parseViewTag(const QDomElement &element, ViewSettings *pViewSettings)
{
    GeneralError parseErr;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cScaleTag)
        {
            parseErr = parseScaleTag(child, &pViewSettings->scaleSettings);
            if (!parseErr.result())
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

    return parseErr;
}

GeneralError ProjectFileParser::parseScaleTag(const QDomElement &element, ScaleSettings *pScaleSettings)
{
    GeneralError parseErr;
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
                pScaleSettings->xAxis.bSliding = true;

                parseErr = parseScaleXAxis(child, pScaleSettings);
                if (!parseErr.result())
                {
                    break;
                }
            }
            else if (!active.compare(ProjectFileDefinitions::cAutoValue, Qt::CaseInsensitive))
            {
                // auto interval mode
                pScaleSettings->xAxis.bSliding = false;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cYaxisTag)
        {
            QString axisString = child.attribute(ProjectFileDefinitions::cAxisAttribute);
            bool bRet = false;
            int axisId = axisString.toInt(&bRet);
            if (bRet == false)
            {
                axisId = UINT32_MAX;
            }
            YAxisSettings* yAxisSettings = (axisId == 1) ? &pScaleSettings->y2Axis: &pScaleSettings->yAxis;

            QString active = child.attribute(ProjectFileDefinitions::cModeAttribute);

            if (!active.compare(ProjectFileDefinitions::cWindowAutoValue, Qt::CaseInsensitive))
            {
                yAxisSettings->bWindowScale = true;
            }
            else if (!active.compare(ProjectFileDefinitions::cMinmaxValue, Qt::CaseInsensitive))
            {
                // min max mode
                yAxisSettings->bMinMax = true;

                parseErr = parseScaleYAxis(child, yAxisSettings);
                if (!parseErr.result())
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

    return parseErr;
}

GeneralError ProjectFileParser::parseScaleXAxis(const QDomElement &element, ScaleSettings *pScaleSettings)
{
    GeneralError parseErr;
    bool bSlidingInterval = false;

    // Check nodes
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        bool bRet;
        if (child.tagName() == ProjectFileDefinitions::cSlidingintervalTag)
        {
            pScaleSettings->xAxis.slidingInterval = child.text().toUInt(&bRet);
            if (bRet)
            {
                bSlidingInterval = true;
            }
            else
            {
                parseErr.reportError(QString("Scale (x-axis) has an incorrect sliding interval. \"%1\" is not a valid number").arg(child.text()));
                break;
            }
        }
        else
        {
            // unknown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    if (parseErr.result() && !bSlidingInterval)
    {
        parseErr.reportError(QString("If x-axis has sliding window scaling then slidinginterval variable should be defined."));
    }

    return parseErr;
}

GeneralError ProjectFileParser::parseScaleYAxis(const QDomElement &element, YAxisSettings *pYAxisSettings)
{
    GeneralError parseErr;
    bool bMin = false;
    bool bMax = false;

    // Check nodes
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        bool bRet;
        if (child.tagName() == ProjectFileDefinitions::cMinTag)
        {
            pYAxisSettings->scaleMin = QLocale().toDouble(child.text(), &bRet);
            if (bRet)
            {
                bMin = true;
            }
            else
            {
                parseErr.reportError(QString("Scale (y-axis) has an incorrect minimum. \"%1\" is not a valid double").arg(child.text()));
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cMaxTag)
        {
            pYAxisSettings->scaleMax = QLocale().toDouble(child.text(), &bRet);
            if (bRet)
            {
                bMax = true;
            }
            else
            {
                parseErr.reportError(QString("Scale (y-axis) has an incorrect maximum. \"%1\" is not a valid double").arg(child.text()));
                break;
            }
        }
        else
        {
            // unknown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    if (parseErr.result())
    {
        if (!bMin)
        {
            parseErr.reportError(QString("If y-axis has min max scaling then min variable should be defined."));
        }
        else if (!bMax)
        {
            parseErr.reportError(QString("If y-axis has min max scaling then max variable should be defined."));
        }
        else
        {
            /* No error */
        }
    }

    return parseErr;
}

GeneralError ProjectFileParser::parseConnectionFields(const QDomElement& child, ConnectionSettings* pConnectionSettings)
{
    bool bRet;
    GeneralError parseErr;
    if (child.tagName() == ProjectFileDefinitions::cConnectionIdTag)
    {
        pConnectionSettings->bConnectionId = true;
        pConnectionSettings->connectionId = child.text().toUInt(&bRet);
        if (!bRet)
        {
            parseErr.reportError(QString("Connection Id (%1) is not a valid number").arg(child.text()));
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
            parseErr.reportError(QString("Port ( %1 ) is not a valid number").arg(child.text()));
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
            parseErr.reportError(QString("Baud rate ( %1 ) is not a valid number").arg(child.text()));
        }
    }
    else if (child.tagName() == ProjectFileDefinitions::cParityTag)
    {
        pConnectionSettings->bParity = true;
        pConnectionSettings->parity = child.text().toUInt(&bRet);
        if (!bRet)
        {
            parseErr.reportError(QString("Parity ( %1 ) is not a valid number").arg(child.text()));
        }
    }
    else if (child.tagName() == ProjectFileDefinitions::cStopBitsTag)
    {
        pConnectionSettings->bStopbits = true;
        pConnectionSettings->stopbits = child.text().toUInt(&bRet);
        if (!bRet)
        {
            parseErr.reportError(QString("Stop bits ( %1 ) is not a valid number").arg(child.text()));
        }
    }
    else if (child.tagName() == ProjectFileDefinitions::cDataBitsTag)
    {
        pConnectionSettings->bDatabits = true;
        pConnectionSettings->databits = child.text().toUInt(&bRet);
        if (!bRet)
        {
            parseErr.reportError(QString("Data bits ( %1 ) is not a valid number").arg(child.text()));
        }
    }
    else if (child.tagName() == ProjectFileDefinitions::cTimeoutTag)
    {
        pConnectionSettings->bTimeout = true;
        pConnectionSettings->timeout = child.text().toUInt(&bRet);
        if (!bRet)
        {
            parseErr.reportError(QString("Timeout ( %1 ) is not a valid number").arg(child.text()));
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

    return parseErr;
}