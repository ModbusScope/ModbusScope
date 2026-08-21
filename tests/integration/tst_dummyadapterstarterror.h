#ifndef TST_DUMMYADAPTERSTARTERROR_H
#define TST_DUMMYADAPTERSTARTERROR_H

#include <QObject>

class SettingsModel;
class AdapterManager;

/*!
 * \brief Integration test for a rejected adapter.start against the real "dummy" adapter binary.
 *
 * Regression coverage for a bug where a syntactically invalid register expression (rejected by
 * adapter.start) caused ModbusScope to force-kill the adapter subprocess and halt polling globally
 * instead of treating it as a per-adapter, non-fatal condition.
 */
class TestDummyAdapterStartError : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void startWithInvalidExpressionKeepsAdapterAliveAndPollable();

private:
    SettingsModel* _pSettingsModel{ nullptr };
    AdapterManager* _pAdapterManager{ nullptr };
};

#endif // TST_DUMMYADAPTERSTARTERROR_H
