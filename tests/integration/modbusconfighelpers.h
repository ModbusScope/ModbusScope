#ifndef MODBUSCONFIGHELPERS_H
#define MODBUSCONFIGHELPERS_H

#include <QJsonArray>
#include <QJsonObject>

/*! \brief Adapter configurations for the dummymodbusadapter, which serves registers from a built-in
 * TCP server. Register 40001 holds 0, 40002 holds 1, and so on (the 0-based register offset).
 */
namespace ModbusConfigHelpers {

constexpr int cServerPort = 5020;
constexpr int cSlaveId = 1;

//! TCP connection to the built-in server, or to another \a port to get an unreachable one.
inline QJsonObject connection(int id, int port = cServerPort, int timeoutMs = 1000)
{
    QJsonObject connection;
    connection["id"] = id;
    connection["type"] = QStringLiteral("tcp");
    connection["ip"] = QStringLiteral("127.0.0.1");
    connection["port"] = port;
    connection["persistent"] = true;
    connection["timeout"] = timeoutMs;
    return connection;
}

inline QJsonObject device(int id, int connectionId, int slaveId = cSlaveId)
{
    QJsonObject device;
    device["id"] = id;
    device["connectionId"] = connectionId;
    device["slaveId"] = slaveId;
    return device;
}

inline QJsonObject config(const QJsonArray& connections, const QJsonArray& devices)
{
    QJsonObject config;
    config["version"] = 1;
    config["general"] = QJsonObject();
    config["connections"] = connections;
    config["devices"] = devices;
    return config;
}

} // namespace ModbusConfigHelpers

#endif // MODBUSCONFIGHELPERS_H
