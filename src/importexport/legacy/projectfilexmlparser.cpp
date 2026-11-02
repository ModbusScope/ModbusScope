
#include "projectfilexmlparser.h"

#include "importexport/projectfiledefinitions.h"

#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QSet>

using ProjectFileData::AdapterFileSettings;
using ProjectFileData::DeviceSettings;
using ProjectFileData::GeneralSettings;
using ProjectFileData::LogSettings;
using ProjectFileData::ProjectSettings;
using ProjectFileData::RegisterSettings;
using ProjectFileData::ScaleSettings;
using ProjectFileData::ScopeSettings;
using ProjectFileData::ViewSettings;
using ProjectFileData::YAxisSettings;

/* Values the old in-app Modbus implementation used when a connection or device field was missing or invalid. */
static const char cTcpType[] = "tcp";
static const char cSerialType[] = "serial";
static const int cDefaultPort = 502;
static const int cDefaultBaudrate = 115200;
static const int cDefaultDataBits = 8;
static const int cDefaultStopBits = 1;
static const quint8 cDefaultSlaveId = 1;
static const quint8 cDefaultConsecutiveMax = 125;

//! Return true when \a baudrate is one of the rates the old serial settings offered.
static bool isSupportedBaudrate(int baudrate)
{
    const QList<int> supportedBaudrates = { 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 };
    return supportedBaudrates.contains(baudrate);
}

/*!
 * \brief Convert a QSerialPort::Parity value to the adapter's parity letter.
 *
 * Old ModbusScope versions only wrote none (0), even (2) and odd (3); any other value fell back
 * to no parity, so it does here too.
 * \param parity  QSerialPort::Parity value from the XML.
 * \return "N", "E" or "O".
 */
static QString parityLetter(quint32 parity)
{
    if (parity == 2)
    {
        return QStringLiteral("E");
    }
    if (parity == 3)
    {
        return QStringLiteral("O");
    }
    return QStringLiteral("N");
}

/*!
 * \brief Fill the fields a legacy connection lacks with the old in-app defaults and name it.
 * \param connection  Connection parsed from the XML.
 * \return The connection with every field the adapter requires.
 */
static QJsonObject completeConnection(const QJsonObject& connection)
{
    QJsonObject result;
    result[ProjectFileDefinitions::cIdJsonKey] = 1;
    result[ProjectFileDefinitions::cConnectionTypeJsonKey] = cTcpType;
    result[ProjectFileDefinitions::cIpTag] = "127.0.0.1";
    result[ProjectFileDefinitions::cPortTag] = cDefaultPort;
    result[ProjectFileDefinitions::cPortNameJsonKey] = "COM1";
    result[ProjectFileDefinitions::cBaudrateTag] = cDefaultBaudrate;
    result[ProjectFileDefinitions::cParityJsonKey] = "N";
    result[ProjectFileDefinitions::cDataBitsTag] = cDefaultDataBits;
    result[ProjectFileDefinitions::cStopBitsTag] = cDefaultStopBits;
    result[ProjectFileDefinitions::cTimeoutTag] = 1000;
    result[ProjectFileDefinitions::cPersistentJsonKey] = true;

    for (auto it = connection.constBegin(); it != connection.constEnd(); ++it)
    {
        result.insert(it.key(), it.value());
    }

    // Without a name the settings page would show the adapter default's name on every connection
    result[ProjectFileDefinitions::cNameJsonKey] =
      QString("Connection %1").arg(result.value(ProjectFileDefinitions::cIdJsonKey).toInt());
    return result;
}

/*!
 * \brief Drop disabled connections that no device uses, and remove the "enabled" flag.
 *
 * Old files always hold all three connections; the adapter has no notion of a disabled one.
 * \param connections  Completed connections.
 * \param devices      Adapter devices, referencing connections by "connectionId".
 * \return The connections to keep.
 */
static QJsonArray removeUnusedDisabledConnections(const QJsonArray& connections, const QJsonArray& devices)
{
    QSet<int> usedIds;
    for (const auto& device : devices)
    {
        usedIds.insert(device.toObject().value(ProjectFileDefinitions::cConnectionIdJsonKey).toInt());
    }

    QJsonArray result;
    for (const auto& connection : connections)
    {
        QJsonObject obj = connection.toObject();
        const bool bEnabled = obj.value(ProjectFileDefinitions::cEnabledAttribute).toBool(true);
        if (!bEnabled && !usedIds.contains(obj.value(ProjectFileDefinitions::cIdJsonKey).toInt()))
        {
            continue;
        }
        obj.remove(ProjectFileDefinitions::cEnabledAttribute);
        result.append(obj);
    }
    return result;
}

/*!
 * \brief Keep only the first connection of each id.
 *
 * Connections without a \c \<connectionid\> all default to the same id.
 * \param connections  Completed connections.
 * \return The connections with repeated ids removed.
 */
static QJsonArray removeDuplicateConnectionIds(const QJsonArray& connections)
{
    QJsonArray result;
    QSet<int> seenIds;
    for (const auto& connection : connections)
    {
        const int id = connection.toObject().value(ProjectFileDefinitions::cIdJsonKey).toInt();
        if (seenIds.contains(id))
        {
            continue;
        }
        seenIds.insert(id);
        result.append(connection);
    }
    return result;
}

ProjectFileXmlParser::ProjectFileXmlParser() : _dataLevel(0)
{
}

/*!
 * \brief Parse a legacy XML MBS project file (data levels 3–5).
 * \param fileContent    Raw file contents.
 * \param pSettings      Output settings structure.
 * \param projectBaseDir Absolute path of the directory containing the project file,
 *                       used to resolve relative log-file paths.
 * \return GeneralError — result() is true on success.
 */
GeneralError ProjectFileXmlParser::parseFile(const QString& fileContent,
                                             ProjectSettings* pSettings,
                                             const QString& projectBaseDir)
{
    GeneralError parseErr;
    _projectBaseDir = projectBaseDir;

    QDomDocument::ParseResult result =
      _domDocument.setContent(fileContent, QDomDocument::ParseOption::UseNamespaceProcessing);
    if (!result)
    {
        parseErr.reportError(QString("Parse error at line %1, column %2:\n%3")
                               .arg(result.errorLine)
                               .arg(result.errorColumn)
                               .arg(result.errorMessage));
        return parseErr;
    }

    QDomElement root = _domDocument.documentElement();
    if (root.tagName() != ProjectFileDefinitions::cModbusScopeTag)
    {
        parseErr.reportError(QString("The file is not a valid ModbusScope project file."));
        return parseErr;
    }

    bool bRet;
    QString strDataLevel = root.attribute(ProjectFileDefinitions::cDatalevelAttribute, "1");
    quint32 datalevel = strDataLevel.toUInt(&bRet);

    if (!bRet)
    {
        parseErr.reportError(QString("Data level (%1) is not a valid number").arg(strDataLevel));
        return parseErr;
    }

    _dataLevel = datalevel;
    if (datalevel < ProjectFileDefinitions::cMinimumDataLevel)
    {
        parseErr.reportError(QString("Data level %1 is not supported. Minimum datalevel is %2.\n\n"
                                     "Try saving the project file with a previous version of ModbusScope.\n\n"
                                     "Project file loading is aborted.")
                               .arg(datalevel)
                               .arg(ProjectFileDefinitions::cMinimumDataLevel));
        return parseErr;
    }

    if (datalevel > ProjectFileDefinitions::cCurrentDataLevel)
    {
        parseErr.reportError(QString("Data level %1 is not supported. Only datalevel %2 or lower is supported.\n\n"
                                     "Try upgrading ModbusScope to the latest version.\n\n"
                                     "Project file loading is aborted.")
                               .arg(datalevel)
                               .arg(ProjectFileDefinitions::cCurrentDataLevel));
        return parseErr;
    }

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

    return parseErr;
}

/*!
 * \brief Parse the \c \<modbus\> section, building both main-app settings and the adapter JSON blob.
 */
GeneralError ProjectFileXmlParser::parseModbusTag(const QDomElement& element, GeneralSettings* pGeneralSettings)
{
    GeneralError parseErr;
    QJsonArray connectionsArray;
    QJsonArray adapterDevicesArray;

    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cConnectionTag)
        {
            QJsonObject connectionJson;

            if (_dataLevel < 5)
            {
                QJsonObject deviceJson;
                DeviceSettings deviceSettings;

                parseErr = parseLegacyConnectionTag(child, &connectionJson, &deviceJson, &deviceSettings);

                if (parseErr.result() && connectionJson.value(ProjectFileDefinitions::cEnabledAttribute).toBool(true))
                {
                    pGeneralSettings->deviceSettings.append(deviceSettings);
                    adapterDevicesArray.append(deviceJson);
                }
            }
            else
            {
                parseErr = parseConnectionTag(child, &connectionJson);
            }

            if (!parseErr.result())
            {
                break;
            }

            connectionsArray.append(connectionJson);
        }
        else if (child.tagName() == ProjectFileDefinitions::cDeviceTag)
        {
            DeviceSettings deviceSettings;
            QJsonObject deviceJson;

            parseErr = parseDeviceTag(child, &deviceSettings, &deviceJson);
            if (!parseErr.result())
            {
                break;
            }

            pGeneralSettings->deviceSettings.append(deviceSettings);
            adapterDevicesArray.append(deviceJson);
        }
        else if (child.tagName() == ProjectFileDefinitions::cLogTag)
        {
            parseErr = parseLogTag(child, &pGeneralSettings->logSettings);
            if (!parseErr.result())
            {
                break;
            }
        }

        child = child.nextSiblingElement();
    }

    if (parseErr.result())
    {
        buildAdapterSettings(connectionsArray, adapterDevicesArray, pGeneralSettings);
    }

    return parseErr;
}

/*!
 * \brief Parse a \c \<connection\> tag (data level 5) into a JSON object for the adapter blob.
 */
GeneralError ProjectFileXmlParser::parseConnectionTag(const QDomElement& element, QJsonObject* pConnectionJson)
{
    GeneralError parseErr;
    QDomElement child = element.firstChildElement();
    while (!child.isNull() && parseErr.result())
    {
        parseErr = parseConnectionFields(child, pConnectionJson);
        child = child.nextSiblingElement();
    }

    return parseErr;
}

/*!
 * \brief Parse a legacy \c \<connection\> tag (data level < 5) which contains both
 *        connection settings and device settings (slaveid, consecutivemax, etc.).
 */
GeneralError ProjectFileXmlParser::parseLegacyConnectionTag(const QDomElement& element,
                                                            QJsonObject* pConnectionJson,
                                                            QJsonObject* pDeviceJson,
                                                            DeviceSettings* pDeviceSettings)
{
    GeneralError parseErr;

    /* In legacy format, each connection implicitly defines a device.
     * The device gets a synthetic deviceId based on the connection id. Both ids are 0-based in the
     * XML and 1-based for the adapter. */
    quint32 connectionId = 0;
    quint8 slaveId = cDefaultSlaveId;
    quint8 consecutiveMax = cDefaultConsecutiveMax;
    bool int32LittleEndian = true;

    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        bool bRet;

        parseErr = parseConnectionFields(child, pConnectionJson);
        if (!parseErr.result())
        {
            break;
        }

        if (child.tagName() == ProjectFileDefinitions::cSlaveIdTag)
        {
            const quint32 val = child.text().toUInt(&bRet);
            if (!bRet)
            {
                parseErr.reportError(QString("Slave id ( %1 ) is not a valid number").arg(child.text()));
                break;
            }
            slaveId = (val >= 1 && val <= 247) ? static_cast<quint8>(val) : cDefaultSlaveId;
        }
        else if (child.tagName() == ProjectFileDefinitions::cConsecutiveMaxTag)
        {
            const quint32 val = child.text().toUInt(&bRet);
            if (!bRet)
            {
                parseErr.reportError(
                  QString("Consecutive register maximum ( %1 ) is not a valid number").arg(child.text()));
                break;
            }
            consecutiveMax = (val >= 1 && val <= 125) ? static_cast<quint8>(val) : cDefaultConsecutiveMax;
        }
        else if (child.tagName() == ProjectFileDefinitions::cInt32LittleEndianTag)
        {
            int32LittleEndian = (child.text().toLower().compare(ProjectFileDefinitions::cTrueValue) == 0);
        }
        else if (child.tagName() == ProjectFileDefinitions::cConnectionIdTag)
        {
            connectionId = child.text().toUInt(&bRet);
            if (!bRet)
            {
                parseErr.reportError(QString("Connection Id (%1) is not a valid number").arg(child.text()));
                break;
            }
        }

        child = child.nextSiblingElement();
    }

    if (parseErr.result())
    {
        /* Build generic device settings for main app */
        pDeviceSettings->bDeviceId = true;
        pDeviceSettings->deviceId = connectionId + 1;

        /* Build adapter device JSON blob */
        (*pDeviceJson)[ProjectFileDefinitions::cIdJsonKey] = static_cast<int>(connectionId + 1);
        (*pDeviceJson)[ProjectFileDefinitions::cConnectionIdJsonKey] = static_cast<int>(connectionId + 1);
        (*pDeviceJson)[ProjectFileDefinitions::cSlaveIdJsonKey] = static_cast<int>(slaveId);
        (*pDeviceJson)[ProjectFileDefinitions::cConsecutiveMaxJsonKey] = static_cast<int>(consecutiveMax);
        (*pDeviceJson)[ProjectFileDefinitions::cInt32LittleEndianJsonKey] = int32LittleEndian;
    }

    return parseErr;
}

/*!
 * \brief Parse a \c \<device\> tag (data level 5) into both generic DeviceSettings and adapter JSON.
 */
GeneralError ProjectFileXmlParser::parseDeviceTag(const QDomElement& element,
                                                  DeviceSettings* pDeviceSettings,
                                                  QJsonObject* pDeviceJson)
{
    GeneralError parseErr;

    quint32 connectionId = 0;
    quint8 slaveId = cDefaultSlaveId;
    quint8 consecutiveMax = cDefaultConsecutiveMax;
    bool int32LittleEndian = true;

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
            connectionId = child.text().toUInt(&bRet);
            if (!bRet)
            {
                parseErr.reportError(QString("Connection Id (%1) is not a valid number").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cSlaveIdTag)
        {
            const quint32 val = child.text().toUInt(&bRet);
            if (!bRet)
            {
                parseErr.reportError(QString("Slave id ( %1 ) is not a valid number").arg(child.text()));
                break;
            }
            slaveId = (val >= 1 && val <= 247) ? static_cast<quint8>(val) : cDefaultSlaveId;
        }
        else if (child.tagName() == ProjectFileDefinitions::cConsecutiveMaxTag)
        {
            const quint32 val = child.text().toUInt(&bRet);
            if (!bRet)
            {
                parseErr.reportError(
                  QString("Consecutive register maximum ( %1 ) is not a valid number").arg(child.text()));
                break;
            }
            consecutiveMax = (val >= 1 && val <= 125) ? static_cast<quint8>(val) : cDefaultConsecutiveMax;
        }
        else if (child.tagName() == ProjectFileDefinitions::cInt32LittleEndianTag)
        {
            int32LittleEndian = (child.text().toLower().compare(ProjectFileDefinitions::cTrueValue) == 0);
        }

        child = child.nextSiblingElement();
    }

    if (parseErr.result())
    {
        (*pDeviceJson)[ProjectFileDefinitions::cIdJsonKey] = static_cast<int>(pDeviceSettings->deviceId);
        (*pDeviceJson)[ProjectFileDefinitions::cConnectionIdJsonKey] = static_cast<int>(connectionId + 1);
        (*pDeviceJson)[ProjectFileDefinitions::cSlaveIdJsonKey] = static_cast<int>(slaveId);
        (*pDeviceJson)[ProjectFileDefinitions::cConsecutiveMaxJsonKey] = static_cast<int>(consecutiveMax);
        (*pDeviceJson)[ProjectFileDefinitions::cInt32LittleEndianJsonKey] = int32LittleEndian;
    }

    return parseErr;
}

/*!
 * \brief Parse the \c \<log\> tag.
 */
GeneralError ProjectFileXmlParser::parseLogTag(const QDomElement& element, LogSettings* pLogSettings)
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
            pLogSettings->bAbsoluteTimes = (child.text().toLower().compare(ProjectFileDefinitions::cTrueValue) == 0);
        }
        else if (child.tagName() == ProjectFileDefinitions::cLogToFileTag)
        {
            parseErr = parseLogToFile(child, pLogSettings);
            if (!parseErr.result())
            {
                break;
            }
        }

        child = child.nextSiblingElement();
    }

    return parseErr;
}

/*!
 * \brief Parse the \c \<logtofile\> element.
 */
GeneralError ProjectFileXmlParser::parseLogToFile(const QDomElement& element, LogSettings* pLogSettings)
{
    GeneralError parseErr;

    QString enabled = element.attribute(ProjectFileDefinitions::cEnabledAttribute, ProjectFileDefinitions::cTrueValue);
    pLogSettings->bLogToFile = (enabled.compare(ProjectFileDefinitions::cTrueValue, Qt::CaseInsensitive) == 0);

    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cFilenameTag)
        {
            QFileInfo fileInfo = (_projectBaseDir.isEmpty() || QFileInfo(child.text()).isAbsolute())
                                   ? QFileInfo(child.text())
                                   : QFileInfo(QDir(_projectBaseDir), child.text());
            bool bValid = true;

            if (!fileInfo.isFile())
            {
                if (fileInfo.exists() || !fileInfo.dir().exists())
                {
                    bValid = false;
                }
            }

            if (bValid)
            {
                pLogSettings->bLogToFileFile = true;
                pLogSettings->logFile = fileInfo.filePath();
            }
        }

        child = child.nextSiblingElement();
    }

    return parseErr;
}

/*!
 * \brief Parse the \c \<scope\> tag containing register definitions.
 */
GeneralError ProjectFileXmlParser::parseScopeTag(const QDomElement& element, ScopeSettings* pScopeSettings)
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

        child = child.nextSiblingElement();
    }

    return parseErr;
}

/*!
 * \brief Parse a single \c \<register\> element.
 */
GeneralError ProjectFileXmlParser::parseRegisterTag(const QDomElement& element, RegisterSettings* pRegisterSettings)
{
    GeneralError parseErr;

    QString active = element.attribute(ProjectFileDefinitions::cActiveAttribute, ProjectFileDefinitions::cTrueValue);
    pRegisterSettings->bActive = (active.compare(ProjectFileDefinitions::cTrueValue, Qt::CaseInsensitive) == 0);

    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cTextTag)
        {
            pRegisterSettings->text = child.text();
        }
        else if (child.tagName() == ProjectFileDefinitions::cColorTag)
        {
            if (QColor::isValidColorName(child.text()))
            {
                pRegisterSettings->bColor = true;
                pRegisterSettings->color = QColor(child.text());
            }
            else
            {
                parseErr.reportError(QString("Color is not a valid color. Did you use correct color format? "
                                             "Expecting #FF0000 (red)"));
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
            else
            {
                parseErr.reportError(QString("Value axis (%1) is not a valid number").arg(child.text()));
                break;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cExpressionTag)
        {
            pRegisterSettings->expression = child.text();
        }

        child = child.nextSiblingElement();
    }

    return parseErr;
}

/*!
 * \brief Parse the \c \<view\> tag.
 */
GeneralError ProjectFileXmlParser::parseViewTag(const QDomElement& element, ViewSettings* pViewSettings)
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

        child = child.nextSiblingElement();
    }

    return parseErr;
}

/*!
 * \brief Parse the \c \<scale\> tag containing x-axis and y-axis settings.
 */
GeneralError ProjectFileXmlParser::parseScaleTag(const QDomElement& element, ScaleSettings* pScaleSettings)
{
    GeneralError parseErr;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cXaxisTag)
        {
            QString mode = child.attribute(ProjectFileDefinitions::cModeAttribute);

            if (mode.compare(ProjectFileDefinitions::cSlidingValue, Qt::CaseInsensitive) == 0)
            {
                pScaleSettings->xAxis.bSliding = true;
                parseErr = parseScaleXAxis(child, pScaleSettings);
                if (!parseErr.result())
                {
                    break;
                }
            }
            else
            {
                pScaleSettings->xAxis.bSliding = false;
            }
        }
        else if (child.tagName() == ProjectFileDefinitions::cYaxisTag)
        {
            QString axisString = child.attribute(ProjectFileDefinitions::cAxisAttribute);
            bool bRet = false;
            int axisId = axisString.toInt(&bRet);
            if (!bRet)
            {
                axisId = 0;
            }
            YAxisSettings* yAxisSettings = (axisId == 1) ? &pScaleSettings->y2Axis : &pScaleSettings->yAxis;

            QString mode = child.attribute(ProjectFileDefinitions::cModeAttribute);

            if (mode.compare(ProjectFileDefinitions::cWindowAutoValue, Qt::CaseInsensitive) == 0)
            {
                yAxisSettings->bWindowScale = true;
            }
            else if (mode.compare(ProjectFileDefinitions::cMinmaxValue, Qt::CaseInsensitive) == 0)
            {
                yAxisSettings->bMinMax = true;
                parseErr = parseScaleYAxis(child, yAxisSettings);
                if (!parseErr.result())
                {
                    break;
                }
            }
        }

        child = child.nextSiblingElement();
    }

    return parseErr;
}

/*!
 * \brief Parse the x-axis sliding interval from a \c \<xaxis\> element.
 */
GeneralError ProjectFileXmlParser::parseScaleXAxis(const QDomElement& element, ScaleSettings* pScaleSettings)
{
    GeneralError parseErr;
    bool bSlidingInterval = false;

    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == ProjectFileDefinitions::cSlidingintervalTag)
        {
            bool bRet;
            pScaleSettings->xAxis.slidingInterval = child.text().toUInt(&bRet);
            if (bRet)
            {
                bSlidingInterval = true;
            }
            else
            {
                parseErr.reportError(QString("Scale (x-axis) has an incorrect sliding interval. "
                                             "\"%1\" is not a valid number")
                                       .arg(child.text()));
                break;
            }
        }

        child = child.nextSiblingElement();
    }

    if (parseErr.result() && !bSlidingInterval)
    {
        parseErr.reportError(
          QString("If x-axis has sliding window scaling then slidinginterval variable should be defined."));
    }

    return parseErr;
}

/*!
 * \brief Parse min/max values from a \c \<yaxis\> element.
 */
GeneralError ProjectFileXmlParser::parseScaleYAxis(const QDomElement& element, YAxisSettings* pYAxisSettings)
{
    GeneralError parseErr;
    bool bMin = false;
    bool bMax = false;

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
                parseErr.reportError(
                  QString("Scale (y-axis) has an incorrect minimum. \"%1\" is not a valid double").arg(child.text()));
                break;
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
                parseErr.reportError(
                  QString("Scale (y-axis) has an incorrect maximum. \"%1\" is not a valid double").arg(child.text()));
                break;
            }
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
    }

    return parseErr;
}

/*!
 * \brief Extract connection fields from a child element into a JSON object for the adapter blob.
 * \return GeneralError — result() is false if a numeric field contains an invalid value.
 */
GeneralError ProjectFileXmlParser::parseConnectionFields(const QDomElement& child, QJsonObject* pConnectionJson)
{
    GeneralError parseErr;
    bool bRet;

    if (child.tagName() == ProjectFileDefinitions::cConnectionIdTag)
    {
        quint32 val = child.text().toUInt(&bRet);
        if (!bRet)
        {
            parseErr.reportError(QString("Connection id (%1) is not a valid number").arg(child.text()));
            return parseErr;
        }
        // XML connection ids start at 0, the adapter's at 1
        (*pConnectionJson)[ProjectFileDefinitions::cIdJsonKey] = static_cast<int>(val + 1);
    }
    else if (child.tagName() == ProjectFileDefinitions::cConnectionEnabledTag)
    {
        (*pConnectionJson)[ProjectFileDefinitions::cEnabledAttribute] =
          (child.text().toLower().compare(ProjectFileDefinitions::cTrueValue) == 0);
    }
    else if (child.tagName() == ProjectFileDefinitions::cConnectionTypeTag)
    {
        const bool bSerial = child.text().compare(cSerialType, Qt::CaseInsensitive) == 0;
        (*pConnectionJson)[ProjectFileDefinitions::cConnectionTypeJsonKey] = bSerial ? cSerialType : cTcpType;
    }
    else if (child.tagName() == ProjectFileDefinitions::cIpTag)
    {
        (*pConnectionJson)[ProjectFileDefinitions::cIpTag] = child.text();
    }
    else if (child.tagName() == ProjectFileDefinitions::cPortTag)
    {
        quint32 val = child.text().toUInt(&bRet);
        if (!bRet)
        {
            parseErr.reportError(QString("Port (%1) is not a valid number").arg(child.text()));
            return parseErr;
        }
        (*pConnectionJson)[ProjectFileDefinitions::cPortTag] =
          (val >= 1 && val <= 65535) ? static_cast<int>(val) : cDefaultPort;
    }
    else if (child.tagName() == ProjectFileDefinitions::cPortNameTag)
    {
        (*pConnectionJson)[ProjectFileDefinitions::cPortNameJsonKey] = child.text();
    }
    else if (child.tagName() == ProjectFileDefinitions::cBaudrateTag)
    {
        quint32 val = child.text().toUInt(&bRet);
        if (!bRet)
        {
            parseErr.reportError(QString("Baudrate (%1) is not a valid number").arg(child.text()));
            return parseErr;
        }
        const int baudrate = static_cast<int>(val);
        (*pConnectionJson)[ProjectFileDefinitions::cBaudrateTag] =
          isSupportedBaudrate(baudrate) ? baudrate : cDefaultBaudrate;
    }
    else if (child.tagName() == ProjectFileDefinitions::cParityTag)
    {
        quint32 val = child.text().toUInt(&bRet);
        if (!bRet)
        {
            parseErr.reportError(QString("Parity (%1) is not a valid number").arg(child.text()));
            return parseErr;
        }
        (*pConnectionJson)[ProjectFileDefinitions::cParityJsonKey] = parityLetter(val);
    }
    else if (child.tagName() == ProjectFileDefinitions::cStopBitsTag)
    {
        quint32 val = child.text().toUInt(&bRet);
        if (!bRet)
        {
            parseErr.reportError(QString("Stop bits (%1) is not a valid number").arg(child.text()));
            return parseErr;
        }
        // 1 = one, 2 = two, 3 = one and a half (QSerialPort::StopBits)
        (*pConnectionJson)[ProjectFileDefinitions::cStopBitsTag] =
          (val >= 1 && val <= 3) ? static_cast<int>(val) : cDefaultStopBits;
    }
    else if (child.tagName() == ProjectFileDefinitions::cDataBitsTag)
    {
        quint32 val = child.text().toUInt(&bRet);
        if (!bRet)
        {
            parseErr.reportError(QString("Data bits (%1) is not a valid number").arg(child.text()));
            return parseErr;
        }
        (*pConnectionJson)[ProjectFileDefinitions::cDataBitsTag] =
          (val >= 5 && val <= 8) ? static_cast<int>(val) : cDefaultDataBits;
    }
    else if (child.tagName() == ProjectFileDefinitions::cTimeoutTag)
    {
        quint32 val = child.text().toUInt(&bRet);
        if (!bRet)
        {
            parseErr.reportError(QString("Timeout (%1) is not a valid number").arg(child.text()));
            return parseErr;
        }
        (*pConnectionJson)[ProjectFileDefinitions::cTimeoutTag] = static_cast<int>(val);
    }
    else if (child.tagName() == ProjectFileDefinitions::cPersistentConnectionTag)
    {
        (*pConnectionJson)[ProjectFileDefinitions::cPersistentJsonKey] =
          (child.text().toLower().compare(ProjectFileDefinitions::cTrueValue) == 0);
    }

    return parseErr;
}

/*!
 * \brief Construct the adapter settings entry from collected connection and device JSON arrays.
 */
void ProjectFileXmlParser::buildAdapterSettings(const QJsonArray& connectionsArray,
                                                const QJsonArray& devicesArray,
                                                GeneralSettings* pGeneralSettings)
{
    AdapterFileSettings adapterSettings;
    adapterSettings.type = "modbus";

    QJsonArray connections;
    for (const auto& connection : connectionsArray)
    {
        connections.append(completeConnection(connection.toObject()));
    }
    connections = removeUnusedDisabledConnections(connections, devicesArray);
    connections = removeDuplicateConnectionIds(connections);

    QJsonObject settingsObj;
    settingsObj[ProjectFileDefinitions::cConnectionsJsonKey] = connections;
    settingsObj[ProjectFileDefinitions::cDevicesJsonKey] = devicesArray;
    adapterSettings.settings = settingsObj;

    pGeneralSettings->adapterList.append(adapterSettings);
    const quint32 newAdapterIndex = static_cast<quint32>(pGeneralSettings->adapterList.size() - 1);

    /* Assign adapter index and type to devices that have not yet been linked to an adapter */
    for (int i = 0; i < pGeneralSettings->deviceSettings.size(); i++)
    {
        if (pGeneralSettings->deviceSettings[i].adapterType.isEmpty())
        {
            pGeneralSettings->deviceSettings[i].adapterId = newAdapterIndex;
            pGeneralSettings->deviceSettings[i].adapterType = "modbus";
        }
    }
}
