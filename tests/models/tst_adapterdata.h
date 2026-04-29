
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

    void settingsModelAdapterDataCreatesEntry();
    void settingsModelAdapterIds();
    void settingsModelRemoveAdapter();

    void dataPointSchemaDefaultEmpty();
    void setAndGetDataPointSchema();
    void settingsModelSetAdapterDataPointSchema();

    void deviceAdapterIdDefaultsToModbus();
    void deviceSetAndGetAdapterId();
    void deviceListForAdapterFiltersCorrectly();
    void deviceListForAdapterUnknownReturnsEmpty();
};

#endif /* TEST_ADAPTERDATA_H__ */
