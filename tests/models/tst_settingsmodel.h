#ifndef TST_SETTINGSMODEL_H
#define TST_SETTINGSMODEL_H

#include <QObject>

class TestSettingsModel : public QObject
{
    Q_OBJECT

private slots:
    void applyDeviceListReplacesList();
    void applyDeviceListEmitsOnceForMultipleChanges();
    void applyDeviceListIsSilentWhenNothingChanged();
    void applyDeviceListDetectsFieldOnlyChange();
    void applyDeviceListEmptyClearsList();
};

#endif // TST_SETTINGSMODEL_H
