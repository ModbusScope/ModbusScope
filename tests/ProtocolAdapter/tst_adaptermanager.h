#ifndef TST_ADAPTERMANAGER_H
#define TST_ADAPTERMANAGER_H

#include <QObject>

class AdapterManager;
class SettingsModel;

class TestAdapterManager : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void diagnosticDebugLevel();
    void diagnosticInfoLevel();
    void diagnosticWarningLevel();
    void diagnosticErrorLevel();
    void diagnosticUnknownLevel();

private:
    SettingsModel* _pSettingsModel{ nullptr };
    AdapterManager* _pAdapterManager{ nullptr };
};

#endif // TST_ADAPTERMANAGER_H
