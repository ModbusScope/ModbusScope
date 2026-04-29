
#include "adapterdata.h"

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

QJsonObject AdapterData::effectiveConfig() const
{
    if (_hasStoredConfig)
    {
        return _currentConfig;
    }
    return _defaults;
}
