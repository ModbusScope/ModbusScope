#ifndef TST_DEVICECONFIGTAB_H
#define TST_DEVICECONFIGTAB_H

#include "models/settingsmodel.h"

#include <QObject>

class TestDeviceConfigTab : public QObject
{
    Q_OBJECT

private slots:
    void comboContainsAllAdapterIds();
    void constructorSelectsCorrectAdapter();
    void valuesReturnsDeviceFieldValues();
    void adapterIdMatchesComboInitially();

private:
    //! Populate \a model with two adapters that each have a minimal device schema.
    void setupTwoAdapters(SettingsModel& model);
};

#endif // TST_DEVICECONFIGTAB_H
