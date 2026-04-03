#ifndef TST_MODBUSPOLL_H
#define TST_MODBUSPOLL_H

#include <QObject>

class ModbusPoll;
class SettingsModel;

class TestModbusPoll : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void diagnosticDebugLevel();
    void diagnosticInfoLevel();
    void diagnosticWarningLevel();
    void diagnosticUnknownLevel();

private:
    SettingsModel* _pSettingsModel{ nullptr };
    ModbusPoll* _pModbusPoll{ nullptr };
};

#endif // TST_MODBUSPOLL_H
