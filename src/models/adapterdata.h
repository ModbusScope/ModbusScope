#ifndef ADAPTERDATA_H
#define ADAPTERDATA_H

#include <QJsonObject>
#include <QString>

/*!
 * \brief Holds adapter describe metadata and opaque configuration.
 *
 * Stores the result of an adapter.describe response (name, version, schema,
 * defaults, capabilities) along with the current adapter configuration.
 * The core application treats the configuration as opaque JSON — it never
 * interprets adapter-specific fields.
 */
class AdapterData
{

public:
    explicit AdapterData();
    ~AdapterData() = default;

    void setName(const QString& name);
    void setVersion(const QString& version);
    void setConfigVersion(int configVersion);
    void setSchema(const QJsonObject& schema);
    void setDefaults(const QJsonObject& defaults);
    void setCapabilities(const QJsonObject& capabilities);
    void setCurrentConfig(const QJsonObject& config);
    void setHasStoredConfig(bool hasStoredConfig);

    QString name() const;
    QString version() const;
    int configVersion() const;
    QJsonObject schema() const;
    QJsonObject defaults() const;
    QJsonObject capabilities() const;
    QJsonObject currentConfig() const;
    bool hasStoredConfig() const;

    /*!
     * \brief Populate describe metadata from an adapter.describe response.
     * \param describeResult The full JSON object returned by adapter.describe.
     */
    void updateFromDescribe(const QJsonObject& describeResult);

    /*!
     * \brief Return the configuration to send to adapter.configure.
     * \return The stored config if available, otherwise the defaults from describe.
     */
    QJsonObject effectiveConfig() const;

private:
    QString _name;
    QString _version;
    int _configVersion{ 0 };
    QJsonObject _schema;
    QJsonObject _defaults;
    QJsonObject _capabilities;
    QJsonObject _currentConfig;
    bool _hasStoredConfig{ false };
};

#endif // ADAPTERDATA_H
