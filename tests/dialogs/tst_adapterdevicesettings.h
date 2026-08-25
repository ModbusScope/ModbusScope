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
    void deviceNamePersistedAfterAcceptAndReopen();
    void addTabDoesNotReuseIdFromAdapterConfig();
    void addTabWithGapAssignsNextAfterMax();
    void closeTabRemovesDeviceFromModelOnAccept();
    void nameChangeDoesNotReachModelUntilAccept();
    void adapterChangeDoesNotReachModelUntilAccept();
    void cancelDiscardsDeviceListEdits();
    void cancelDiscardsDeviceFieldEdits();
    void openingDialogKeepsDeviceNoAdapterDeclares();
    void openingDialogKeepsDevicesOfUndescribedAdapter();
    void adapterDescribingWhileOpenIsNotOverwrittenOnAccept();
    void acceptEmitsDeviceListChangedOnce();
    void multipleAdaptersWithDevices();
    void cancelAndReopenDoesNotLeakDeviceIds();
    void acceptValuesClearsDevicesForEmptiedAdapter();
    void twoAdaptersWithSameDefaultDeviceIdShowsSingleTab();
    void adapterDeclaringSameDeviceIdTwiceShowsSingleTab();
    void existingDeviceAdapterIdMatchesConfigOnOpen();
    void sharedDefaultDeviceIdReconciledOnDescribeWithoutOpeningDialog();
    void explicitDeviceAssignmentSurvivesLaterAdapterRedescribe();
    void dialogBuildsTabFromReconciledOwnerNotFirstAdapter();
    void reassigningExistingDeviceOwnerEmitsDeviceListChanged();
    void addTabAfterDefaultDeviceKeepsOrder();
    void reopenAfterAdapterSwitchPreservesDeviceOrder();
    void invalidIdTabSortsAfterValidIntMaxIdTab();
    void invalidIdTabValuesOmitFabricatedId();
    void addTabDefaultsToModbusEvenWhenNotFirstAlphabetically();
    void initialDeviceReconciliationPrefersModbusOverDiscoveryOrder();
    void addButtonRemainsAvailableWhenAdapterOverLimit();
    void warningLabelShowsForOverLimitAdapter();
    void warningLabelShowsForOverCapabilitiesMaxDevices();
    void addingDeviceForModbusNotBlockedByOtherAdapterOverLimit();
    void doesNotDropDevicesWhenAdapterConfigMatchesDeviceList();

private:
    //! Populate \a model with an adapter that has a minimal device schema and
    //! the given \a devices array stored as current config.
    void setupAdapter(SettingsModel& model, const QString& adapterId, const QJsonArray& devices);
};

#endif // TST_ADAPTERDEVICESETTINGS_H
