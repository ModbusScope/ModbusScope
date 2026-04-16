#ifndef TST_ADAPTERDEVICESETTINGS_H
#define TST_ADAPTERDEVICESETTINGS_H

#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QObject>

class TestAdapterDeviceSettings : public QObject
{
    Q_OBJECT

private slots:
    void noAdapterShowsLabel();
    void devicesPopulateTabsFromConfig();
    void deviceModelNameUsedAsTabTitle();
    void missingNameFallsBackToDeviceN();
    void acceptValuesSavesToAdapterConfig();
    void acceptValuesSavesDeviceNameToModel();
    void addTabUsesDeviceDefaults();
    void addTabIncrementsDeviceId();
    void deviceIdPreservedWhenAdapterChanged();

private:
    //! Populate \a model with an adapter that has a minimal device schema and
    //! the given \a devices array stored as current config.
    void setupAdapter(SettingsModel& model, const QString& adapterId, const QJsonArray& devices);
};

#endif // TST_ADAPTERDEVICESETTINGS_H
