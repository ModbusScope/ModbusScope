#ifndef ADAPTERDISCOVERY_H
#define ADAPTERDISCOVERY_H

#include <QList>
#include <QString>

/*!
 * \brief Holds the identity and binary path for a discovered adapter.
 */
struct AdapterInfo
{
    QString id;
    QString binaryPath;
};

/*!
 * \brief Scans a directory for adapter binaries and returns their identities.
 *
 * Naming convention: \c {id}adapter on Linux, \c {id}adapter.exe on Windows.
 * For example \c modbusadapter yields id \c "modbus".
 */
class AdapterDiscovery
{
public:
    static QList<AdapterInfo> discover(const QString& appDir);
};

#endif // ADAPTERDISCOVERY_H
