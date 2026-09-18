#ifndef TST_MODBUSADAPTERQUALITY_H
#define TST_MODBUSADAPTERQUALITY_H

#include <QObject>

class SettingsModel;
class AdapterManager;

/*!
 * \brief Integration test for protocol-2 data quality against the real Modbus adapter binary.
 *
 * The Modbus adapter has no source of quality flags: it must declare no quality capability, and
 * every data point must come back as plain Good or Invalid without flags.
 */
class TestModbusAdapterQuality : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void describeAdvertisesProtocolVersionWithoutQualityCapability();
    void readDataIsGoodOrInvalidWithoutFlags();

private:
    SettingsModel* _pSettingsModel{ nullptr };
    AdapterManager* _pAdapterManager{ nullptr };
};

#endif // TST_MODBUSADAPTERQUALITY_H
