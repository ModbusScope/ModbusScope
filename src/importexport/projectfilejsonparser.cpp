
#include "projectfilejsonparser.h"

#include "importexport/projectfiledefinitions.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLocale>

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

ProjectFileJsonParser::ProjectFileJsonParser()
{
}

/*!
 * \brief Parse a JSON MBS project file (version 6+).
 * \param fileContent Raw file contents.
 * \param pSettings   Output settings structure.
 * \return GeneralError — result() is true on success.
 */
GeneralError ProjectFileJsonParser::parseFile(const QString& fileContent, ProjectSettings* pSettings)
{
    GeneralError parseErr;

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(fileContent.toUtf8(), &jsonError);

    if (doc.isNull())
    {
        parseErr.reportError(QString("JSON parse error at offset %1: %2")
                               .arg(jsonError.offset)
                               .arg(jsonError.errorString()));
        return parseErr;
    }

    if (!doc.isObject())
    {
        parseErr.reportError(QString("The file is not a valid MBS project file (expected a JSON object at root)."));
        return parseErr;
    }

    QJsonObject root = doc.object();

    if (!root.contains(ProjectFileDefinitions::cVersionKey))
    {
        parseErr.reportError(QString("Missing required \"%1\" key in project file.").arg(ProjectFileDefinitions::cVersionKey));
        return parseErr;
    }

    QJsonValue versionVal = root[ProjectFileDefinitions::cVersionKey];
    if (!versionVal.isDouble())
    {
        parseErr.reportError(QString("The \"%1\" value is not a valid number.").arg(ProjectFileDefinitions::cVersionKey));
        return parseErr;
    }

    quint32 version = static_cast<quint32>(versionVal.toInt());
    if (version < ProjectFileDefinitions::cMinimumJsonVersion)
    {
        parseErr.reportError(
          QString("Version %1 is not supported. Minimum supported version is %2.\n\nProject file loading is aborted.")
            .arg(version)
            .arg(ProjectFileDefinitions::cMinimumJsonVersion));
        return parseErr;
    }

    if (version > ProjectFileDefinitions::cCurrentJsonVersion)
    {
        parseErr.reportError(
          QString("Version %1 is not supported. Only version %2 or lower is supported.\n\n"
                  "Try upgrading ModbusScope to the latest version.\n\nProject file loading is aborted.")
            .arg(version)
            .arg(ProjectFileDefinitions::cCurrentJsonVersion));
        return parseErr;
    }

    if (root.contains(ProjectFileDefinitions::cAdaptersKey) && root[ProjectFileDefinitions::cAdaptersKey].isArray())
    {
        parseErr = parseAdapters(root[ProjectFileDefinitions::cAdaptersKey].toArray(), &pSettings->general);
        if (!parseErr.result())
        {
            return parseErr;
        }
    }

    if (root.contains(ProjectFileDefinitions::cDevicesJsonKey) && root[ProjectFileDefinitions::cDevicesJsonKey].isArray())
    {
        parseErr = parseDevices(root[ProjectFileDefinitions::cDevicesJsonKey].toArray(), &pSettings->general);
        if (!parseErr.result())
        {
            return parseErr;
        }
    }

    if (root.contains(ProjectFileDefinitions::cLogTag) && root[ProjectFileDefinitions::cLogTag].isObject())
    {
        parseErr = parseLog(root[ProjectFileDefinitions::cLogTag].toObject(), &pSettings->general.logSettings);
        if (!parseErr.result())
        {
            return parseErr;
        }
    }

    if (root.contains(ProjectFileDefinitions::cScopeTag) && root[ProjectFileDefinitions::cScopeTag].isArray())
    {
        parseErr = parseScope(root[ProjectFileDefinitions::cScopeTag].toArray(), &pSettings->scope);
        if (!parseErr.result())
        {
            return parseErr;
        }
    }

    if (root.contains(ProjectFileDefinitions::cViewTag) && root[ProjectFileDefinitions::cViewTag].isObject())
    {
        parseErr = parseView(root[ProjectFileDefinitions::cViewTag].toObject(), &pSettings->view);
        if (!parseErr.result())
        {
            return parseErr;
        }
    }

    return parseErr;
}

/*!
 * \brief Parse the top-level "adapters" array.
 *
 * Each entry's "settings" object is stored as an opaque QJsonObject without
 * further interpretation — the adapter is responsible for validation.
 */
GeneralError ProjectFileJsonParser::parseAdapters(const QJsonArray& adaptersArray, GeneralSettings* pGeneralSettings)
{
    GeneralError parseErr;

    for (const QJsonValue& val : adaptersArray)
    {
        if (!val.isObject())
        {
            parseErr.reportError(QString("Each entry in \"%1\" must be a JSON object.").arg(ProjectFileDefinitions::cAdaptersKey));
            break;
        }

        QJsonObject adapterObj = val.toObject();
        AdapterFileSettings adapterSettings;

        if (adapterObj.contains(ProjectFileDefinitions::cAdapterTypeKey))
        {
            adapterSettings.type = adapterObj[ProjectFileDefinitions::cAdapterTypeKey].toString();
        }

        if (adapterObj.contains(ProjectFileDefinitions::cAdapterSettingsKey) && adapterObj[ProjectFileDefinitions::cAdapterSettingsKey].isObject())
        {
            adapterSettings.settings = adapterObj[ProjectFileDefinitions::cAdapterSettingsKey].toObject();
        }

        pGeneralSettings->adapterList.append(adapterSettings);
    }

    return parseErr;
}

/*!
 * \brief Parse the top-level "devices" array (generic device definitions only).
 *
 * Each entry provides the device's generic identity (id, name) and a reference
 * to its adapter (adapterId, adapter.type). Adapter-specific device config lives
 * inside the corresponding adapter's settings blob.
 */
GeneralError ProjectFileJsonParser::parseDevices(const QJsonArray& devicesArray, GeneralSettings* pGeneralSettings)
{
    GeneralError parseErr;

    for (const QJsonValue& val : devicesArray)
    {
        if (!val.isObject())
        {
            parseErr.reportError(QString("Each entry in \"%1\" must be a JSON object.").arg(ProjectFileDefinitions::cDevicesJsonKey));
            break;
        }

        QJsonObject deviceObj = val.toObject();
        DeviceSettings deviceSettings;
        bool bRet = false;

        if (deviceObj.contains(ProjectFileDefinitions::cDeviceIdTag))
        {
            deviceSettings.bDeviceId = true;
            deviceSettings.deviceId = static_cast<deviceId_t>(deviceObj[ProjectFileDefinitions::cDeviceIdTag].toInt(0));
        }

        if (deviceObj.contains(ProjectFileDefinitions::cDeviceNameTag))
        {
            deviceSettings.bName = true;
            deviceSettings.name = deviceObj[ProjectFileDefinitions::cDeviceNameTag].toString();
        }

        if (deviceObj.contains(ProjectFileDefinitions::cAdapterIdKey))
        {
            int adapterIdVal = deviceObj[ProjectFileDefinitions::cAdapterIdKey].toInt(-1);
            if (adapterIdVal < 0)
            {
                parseErr.reportError(QString("Device \"%1\" is not a valid non-negative integer.").arg(ProjectFileDefinitions::cAdapterIdKey));
                break;
            }
            deviceSettings.adapterId = static_cast<quint32>(adapterIdVal);
            bRet = true;
        }

        Q_UNUSED(bRet)

        if (deviceObj.contains(ProjectFileDefinitions::cAdapterKey) && deviceObj[ProjectFileDefinitions::cAdapterKey].isObject())
        {
            QJsonObject adapterObj = deviceObj[ProjectFileDefinitions::cAdapterKey].toObject();
            if (adapterObj.contains(ProjectFileDefinitions::cAdapterTypeKey))
            {
                deviceSettings.adapterType = adapterObj[ProjectFileDefinitions::cAdapterTypeKey].toString();
            }
        }

        pGeneralSettings->deviceSettings.append(deviceSettings);
    }

    return parseErr;
}

/*!
 * \brief Parse the "log" object.
 */
GeneralError ProjectFileJsonParser::parseLog(const QJsonObject& logObject, LogSettings* pLogSettings)
{
    GeneralError parseErr;

    if (logObject.contains(ProjectFileDefinitions::cPollTimeTag))
    {
        int pollTime = logObject[ProjectFileDefinitions::cPollTimeTag].toInt(-1);
        if (pollTime < 0)
        {
            parseErr.reportError(QString("Poll time (%1) is not a valid number.").arg(logObject[ProjectFileDefinitions::cPollTimeTag].toString()));
            return parseErr;
        }
        pLogSettings->bPollTime = true;
        pLogSettings->pollTime = static_cast<quint32>(pollTime);
    }

    if (logObject.contains(ProjectFileDefinitions::cAbsoluteTimesTag))
    {
        pLogSettings->bAbsoluteTimes = logObject[ProjectFileDefinitions::cAbsoluteTimesTag].toBool(false);
    }

    if (logObject.contains(ProjectFileDefinitions::cLogToFileTag) && logObject[ProjectFileDefinitions::cLogToFileTag].isObject())
    {
        QJsonObject logToFileObj = logObject[ProjectFileDefinitions::cLogToFileTag].toObject();

        pLogSettings->bLogToFile = logToFileObj[ProjectFileDefinitions::cEnabledAttribute].toBool(false);

        if (logToFileObj.contains(ProjectFileDefinitions::cFilenameTag))
        {
            QString filename = logToFileObj[ProjectFileDefinitions::cFilenameTag].toString();
            if (!filename.isEmpty())
            {
                QFileInfo fileInfo(filename);
                bool bValid = true;

                if (!fileInfo.isFile())
                {
                    if (fileInfo.exists())
                    {
                        bValid = false;
                        parseErr.reportError(
                          QString("Log file path (%1) already exists, but it is not a file. Log file is set to default.")
                            .arg(fileInfo.filePath()));
                    }
                    else if (!fileInfo.dir().exists())
                    {
                        bValid = false;
                        parseErr.reportError(
                          QString("Log file path (parent directory) does not exist (%1). Log file is set to default.")
                            .arg(fileInfo.filePath()));
                    }
                }

                if (bValid)
                {
                    pLogSettings->bLogToFileFile = true;
                    pLogSettings->logFile = fileInfo.filePath();
                }
            }
        }
    }

    return parseErr;
}

/*!
 * \brief Parse the "scope" array of register definitions.
 */
GeneralError ProjectFileJsonParser::parseScope(const QJsonArray& scopeArray, ScopeSettings* pScopeSettings)
{
    GeneralError parseErr;

    for (const QJsonValue& val : scopeArray)
    {
        if (!val.isObject())
        {
            continue;
        }

        QJsonObject regObj = val.toObject();
        RegisterSettings reg;

        reg.bActive = regObj[ProjectFileDefinitions::cActiveAttribute].toBool(true);
        reg.text = regObj[ProjectFileDefinitions::cTextTag].toString();
        reg.expression = regObj[ProjectFileDefinitions::cExpressionTag].toString();

        if (regObj.contains(ProjectFileDefinitions::cColorTag))
        {
            QString colorStr = regObj[ProjectFileDefinitions::cColorTag].toString();
            if (QColor::isValidColorName(colorStr))
            {
                reg.bColor = true;
                reg.color = QColor(colorStr);
            }
            else
            {
                parseErr.reportError(QString("Color \"%1\" is not a valid color. Expecting format #FF0000 (red).").arg(colorStr));
                break;
            }
        }

        if (regObj.contains(ProjectFileDefinitions::cValueAxisTag))
        {
            reg.valueAxis = static_cast<quint32>(regObj[ProjectFileDefinitions::cValueAxisTag].toInt(0));
        }

        pScopeSettings->registerList.append(reg);
    }

    return parseErr;
}

/*!
 * \brief Parse the "view" object containing scale settings.
 */
GeneralError ProjectFileJsonParser::parseView(const QJsonObject& viewObject, ViewSettings* pViewSettings)
{
    GeneralError parseErr;

    if (!viewObject.contains(ProjectFileDefinitions::cScaleTag) || !viewObject[ProjectFileDefinitions::cScaleTag].isObject())
    {
        return parseErr;
    }

    QJsonObject scaleObj = viewObject[ProjectFileDefinitions::cScaleTag].toObject();

    if (scaleObj.contains(ProjectFileDefinitions::cXaxisTag) && scaleObj[ProjectFileDefinitions::cXaxisTag].isObject())
    {
        parseErr = parseScaleXAxis(scaleObj[ProjectFileDefinitions::cXaxisTag].toObject(), &pViewSettings->scaleSettings);
        if (!parseErr.result())
        {
            return parseErr;
        }
    }

    if (scaleObj.contains(ProjectFileDefinitions::cYaxisTag) && scaleObj[ProjectFileDefinitions::cYaxisTag].isArray())
    {
        for (const QJsonValue& yval : scaleObj[ProjectFileDefinitions::cYaxisTag].toArray())
        {
            if (!yval.isObject())
            {
                continue;
            }
            parseErr = parseScaleYAxis(yval.toObject(), &pViewSettings->scaleSettings);
            if (!parseErr.result())
            {
                return parseErr;
            }
        }
    }

    return parseErr;
}

/*!
 * \brief Parse the x-axis scale object.
 */
GeneralError ProjectFileJsonParser::parseScaleXAxis(const QJsonObject& xaxisObject, ScaleSettings* pScaleSettings)
{
    GeneralError parseErr;

    QString mode = xaxisObject[ProjectFileDefinitions::cModeAttribute].toString();

    if (mode.compare(ProjectFileDefinitions::cSlidingValue, Qt::CaseInsensitive) == 0)
    {
        pScaleSettings->xAxis.bSliding = true;

        if (!xaxisObject.contains(ProjectFileDefinitions::cSlidingintervalTag))
        {
            parseErr.reportError(QString("If x-axis has sliding window scaling then \"%1\" must be defined.")
                                   .arg(ProjectFileDefinitions::cSlidingintervalTag));
            return parseErr;
        }

        int interval = xaxisObject[ProjectFileDefinitions::cSlidingintervalTag].toInt(-1);
        if (interval < 0)
        {
            parseErr.reportError(
              QString("Scale (x-axis) has an incorrect sliding interval. Value is not a valid number."));
            return parseErr;
        }
        pScaleSettings->xAxis.slidingInterval = static_cast<quint32>(interval);
    }
    else
    {
        pScaleSettings->xAxis.bSliding = false;
    }

    return parseErr;
}

/*!
 * \brief Parse one y-axis scale object (axis 0 or axis 1).
 */
GeneralError ProjectFileJsonParser::parseScaleYAxis(const QJsonObject& yaxisObject, ScaleSettings* pScaleSettings)
{
    GeneralError parseErr;

    int axisId = yaxisObject[ProjectFileDefinitions::cAxisAttribute].toInt(0);
    YAxisSettings* pAxis = (axisId == 1) ? &pScaleSettings->y2Axis : &pScaleSettings->yAxis;

    QString mode = yaxisObject[ProjectFileDefinitions::cModeAttribute].toString();

    if (mode.compare(ProjectFileDefinitions::cWindowAutoValue, Qt::CaseInsensitive) == 0)
    {
        pAxis->bWindowScale = true;
    }
    else if (mode.compare(ProjectFileDefinitions::cMinmaxValue, Qt::CaseInsensitive) == 0)
    {
        pAxis->bMinMax = true;

        if (!yaxisObject.contains(ProjectFileDefinitions::cMinTag) || !yaxisObject.contains(ProjectFileDefinitions::cMaxTag))
        {
            parseErr.reportError(QString("If y-axis has min-max scaling then both \"%1\" and \"%2\" must be defined.")
                                   .arg(ProjectFileDefinitions::cMinTag)
                                   .arg(ProjectFileDefinitions::cMaxTag));
            return parseErr;
        }

        bool bMinOk = yaxisObject[ProjectFileDefinitions::cMinTag].isDouble();
        bool bMaxOk = yaxisObject[ProjectFileDefinitions::cMaxTag].isDouble();

        if (!bMinOk)
        {
            parseErr.reportError(QString("Scale (y-axis) has an incorrect minimum. Value is not a valid number."));
            return parseErr;
        }
        if (!bMaxOk)
        {
            parseErr.reportError(QString("Scale (y-axis) has an incorrect maximum. Value is not a valid number."));
            return parseErr;
        }

        pAxis->scaleMin = yaxisObject[ProjectFileDefinitions::cMinTag].toDouble();
        pAxis->scaleMax = yaxisObject[ProjectFileDefinitions::cMaxTag].toDouble();
    }
    else
    {
        /* auto mode — nothing to set */
    }

    return parseErr;
}
