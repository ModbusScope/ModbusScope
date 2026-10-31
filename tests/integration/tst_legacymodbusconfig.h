#ifndef TST_LEGACYMODBUSCONFIG_H
#define TST_LEGACYMODBUSCONFIG_H

#include <QObject>

class AdapterClient;

/*!
 * \brief Integration test: legacy XML project files configure the real Modbus adapter.
 *
 * Parses each legacy XML fixture with ProjectFileXmlParser and sends the resulting adapter
 * config, exactly as ModbusScope would put it on the wire, to the real modbusadapter binary.
 * The adapter must accept it with adapter.configure.
 */
class TestLegacyModbusConfig : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void cleanup();

    void legacyProjectConfiguresAdapter_data();
    void legacyProjectConfiguresAdapter();

private:
    AdapterClient* _pClient{ nullptr };
};

#endif // TST_LEGACYMODBUSCONFIG_H
