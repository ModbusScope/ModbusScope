#include "models/settingsmodel.h"

#include <QObject>

class TestAdapterDeviceSettings : public QObject
{
    Q_OBJECT

private slots:
    void noAdapterShowsLabel();
    void devicesPopulateTabsFromConfig();
    void deviceNameUsedAsTabTitle();
    void missingNameFallsBackToDeviceN();
    void acceptValuesSavesToAdapterConfig();

private:
    //! Populate \a model with an adapter that has a minimal device schema and
    //! the given \a devices array stored as current config.
    void setupAdapter(SettingsModel& model, const QString& adapterId, const QJsonArray& devices);
};
