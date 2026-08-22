#ifndef TST_DUMMYDEVICELIMIT_H
#define TST_DUMMYDEVICELIMIT_H

#include <QObject>

class SettingsModel;
class AdapterManager;

class TestDummyDeviceLimit : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void sessionStartsWithTwoDevicesOverLimit();

private:
    SettingsModel* _pSettingsModel{ nullptr };
    AdapterManager* _pAdapterManager{ nullptr };
};

#endif // TST_DUMMYDEVICELIMIT_H
