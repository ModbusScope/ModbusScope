#ifndef ADAPTERDATA_H
#define ADAPTERDATA_H

#include <QJsonObject>
#include <QString>

/*!
 * \brief Holds adapter describe metadata, register schema, and opaque configuration.
 *
 * Stores the result of an adapter.describe response (name, version, schema,
 * defaults, capabilities), the register schema from adapter.registerSchema,
 * and the current adapter configuration.
 * The core application treats all adapter-specific JSON as opaque — it never
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
    void setRegisterSchema(const QJsonObject& schema);

    QString name() const;
    QString version() const;
    int configVersion() const;
    QJsonObject schema() const;
    QJsonObject defaults() const;
    QJsonObject capabilities() const;
    QJsonObject currentConfig() const;
    bool hasStoredConfig() const;
    QJsonObject registerSchema() const;

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
    QJsonObject _registerSchema;
};

#endif // ADAPTERDATA_H
