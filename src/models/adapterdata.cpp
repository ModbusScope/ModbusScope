
#include "adapterdata.h"

#include <climits>

AdapterData::AdapterData() = default;

void AdapterData::setName(const QString& name)
{
    _name = name;
}

void AdapterData::setVersion(const QString& version)
{
    _version = version;
}

void AdapterData::setConfigVersion(int configVersion)
{
    _configVersion = configVersion;
}

void AdapterData::setSchema(const QJsonObject& schema)
{
    _schema = schema;
}

void AdapterData::setDefaults(const QJsonObject& defaults)
{
    _defaults = defaults;
}

void AdapterData::setCapabilities(const QJsonObject& capabilities)
{
    _capabilities = capabilities;
}

void AdapterData::setCurrentConfig(const QJsonObject& config)
{
    _currentConfig = config;
}

void AdapterData::setHasStoredConfig(bool hasStoredConfig)
{
    _hasStoredConfig = hasStoredConfig;
}

//! \brief Sets the data point schema received from adapter.dataPointSchema.
//! \param schema The full data point schema JSON object.
void AdapterData::setDataPointSchema(const QJsonObject& schema)
{
    _dataPointSchema = schema;
}

QString AdapterData::name() const
{
    return _name;
}

QString AdapterData::version() const
{
    return _version;
}

int AdapterData::configVersion() const
{
    return _configVersion;
}

QJsonObject AdapterData::schema() const
{
    return _schema;
}

QJsonObject AdapterData::defaults() const
{
    return _defaults;
}

QJsonObject AdapterData::capabilities() const
{
    return _capabilities;
}

QJsonObject AdapterData::currentConfig() const
{
    return _currentConfig;
}

bool AdapterData::hasStoredConfig() const
{
    return _hasStoredConfig;
}

//! \brief Returns the data point schema set via setDataPointSchema.
//! \return The data point schema JSON object, or an empty object if not yet set.
QJsonObject AdapterData::dataPointSchema() const
{
    return _dataPointSchema;
}

void AdapterData::updateFromDescribe(const QJsonObject& describeResult)
{
    _name = describeResult.value("name").toString();
    _version = describeResult.value("version").toString();
    _configVersion = describeResult.value("configVersion").toInt(0);
    _schema = describeResult.value("schema").toObject();
    _defaults = describeResult.value("defaults").toObject();
    _capabilities = describeResult.value("capabilities").toObject();
}

int AdapterData::maxDevicesFromSchema() const
{
    const QJsonObject devicesSchema = _schema.value("properties").toObject().value("devices").toObject();
    if (devicesSchema.contains("maxItems"))
    {
        return devicesSchema.value("maxItems").toInt(INT_MAX);
    }
    return INT_MAX;
}

QJsonObject AdapterData::effectiveConfig() const
{
    if (!_hasStoredConfig)
    {
        return _defaults;
    }

    /* Use defaults as the base so that keys added to the adapter after a project file was
     * saved (e.g. 'general') are automatically filled in rather than left absent.
     * This is a shallow merge: each top-level key in _currentConfig replaces the
     * corresponding key in result entirely. Nested objects are not merged — if a stored
     * key holds a QJsonObject, the whole sub-object is taken from _currentConfig and none
     * of the default's nested keys are backfilled. A recursive merge would be needed if
     * per-key deep backfilling is ever required (see result/_currentConfig/_defaults here). */
    QJsonObject result = _defaults;
    for (auto it = _currentConfig.constBegin(); it != _currentConfig.constEnd(); ++it)
    {
        result[it.key()] = it.value();
    }
    return result;
}
