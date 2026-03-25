
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
};

#endif /* TEST_ADAPTERDATA_H__ */
