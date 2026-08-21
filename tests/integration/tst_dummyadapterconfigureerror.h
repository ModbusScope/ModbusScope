#ifndef TST_DUMMYADAPTERCONFIGUREERROR_H
#define TST_DUMMYADAPTERCONFIGUREERROR_H

#include <QObject>

class SettingsModel;
class AdapterManager;

/*!
 * \brief Integration test for a rejected adapter.configure against the real "dummy" adapter binary.
 *
 * Regression coverage for a bug where a device count exceeding the adapter's actual device limit
 * caused ModbusScope to force-kill the adapter subprocess and emit a fatal sessionError instead of
 * treating it as a per-adapter, non-fatal condition — the same failure mode already fixed for a
 * rejected adapter.start in TestDummyAdapterStartError, recurring one RPC step earlier.
 */
class TestDummyAdapterConfigureError : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void configureOverDeviceLimitKeepsAdapterAliveAndPollable();

private:
    SettingsModel* _pSettingsModel{ nullptr };
    AdapterManager* _pAdapterManager{ nullptr };
};

#endif // TST_DUMMYADAPTERCONFIGUREERROR_H
