#ifndef TST_PROJECTFILEHANDLER_H
#define TST_PROJECTFILEHANDLER_H

#include <QObject>

class TestProjectFileHandler : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void applyDeviceSettingsAppliesAdapterId();
    void applyDeviceSettingsAppliesName();
    void applyDeviceSettingsMultipleAdapters();
    void applyDeviceSettingsEmptyListClearsModel();
    void applyDeviceSettingsClearsPreviousDevices();

    void loadSetsAdapterConfigInSettingsModel();
    void savePreservesAdapterConfigAfterDialogAccept();
    void roundTripPreservesAdapterConfig();
};

#endif /* TST_PROJECTFILEHANDLER_H */
