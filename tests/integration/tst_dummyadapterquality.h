#ifndef TST_DUMMYADAPTERQUALITY_H
#define TST_DUMMYADAPTERQUALITY_H

#include <QObject>

class SettingsModel;
class AdapterManager;

/*!
 * \brief Integration test for protocol-2 data quality against the real "dummy" adapter binary.
 *
 * The dummy adapter reports a deterministic quality per register address (address % 5), so a
 * single read proves every DataQuality state and flag survives the real wire encoding and
 * AdapterClient's strict decode.
 */
class TestDummyAdapterQuality : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void describeAdvertisesProtocolVersionAndQualityCapability();
    void readDataCarriesAllStatesAndFlags();

private:
    SettingsModel* _pSettingsModel{ nullptr };
    AdapterManager* _pAdapterManager{ nullptr };
};

#endif // TST_DUMMYADAPTERQUALITY_H
