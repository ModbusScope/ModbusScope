
#ifndef TEST_ADAPTERDATA_H__
#define TEST_ADAPTERDATA_H__

#include <QObject>

class TestAdapterData : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void defaultConstruction();
    void updateFromDescribe();
    void updateFromDescribeMissingFields();
    void effectiveConfigReturnsDefaults();
    void effectiveConfigReturnsStoredConfig();
    void effectiveConfigFillsMissingKeysFromDefaults();

    void settingsModelAdapterDataCreatesEntry();
    void settingsModelAdapterIds();
    void settingsModelRemoveAdapter();

    void dataPointSchemaDefaultEmpty();
    void setAndGetDataPointSchema();
    void settingsModelSetAdapterDataPointSchema();

    void deviceAdapterIdDefaultsToModbus();
    void deviceSetAndGetAdapterId();

    void maxDevicesFromSchemaReturnsIntMaxWhenAbsent();
    void maxDevicesFromSchemaReturnsValue();
    void maxDevicesFromSchemaReturnsIntMaxWhenNegative();
    void maxDevicesFromSchemaReturnsZeroWhenExplicit();

    void maxDevicesFromCapabilitiesReturnsIntMaxWhenAbsent();
    void maxDevicesFromCapabilitiesReturnsValue();
    void maxDevicesFromCapabilitiesReturnsIntMaxWhenNegative();
    void maxDevicesFromCapabilitiesReturnsZeroWhenExplicit();

    void maxDevicesReturnsSmallerOfSchemaAndCapabilities();

    void maxRegistersReturnsIntMaxWhenAbsent();
    void maxRegistersReturnsValue();
    void maxRegistersReturnsIntMaxWhenNegative();
    void maxRegistersReturnsZeroWhenExplicit();

    void configForWireCapsDevicesToMaxItems();
    void configForWireReturnsAllDevicesWhenNoLimit();
    void configForWireDoesNotTruncateWhenMaxItemsNegative();
    void configForWireRemovesAllDevicesWhenMaxItemsZero();
    void configForWireCapsDevicesToCapabilitiesMaxDevices();
    void configForWireRemovesDuplicateDeviceIds();
    void configForWireDedupesBeforeTruncating();
    void configForWireKeepsDevicesWithoutId();

    void isMbcCompatibleTrue();
    void isMbcCompatibleFalse();
    void settingsModelIsMbcCompatible();

    void licenseFieldParsed();
    void updateFromDescribeMissingLicense();
};

#endif /* TEST_ADAPTERDATA_H__ */
