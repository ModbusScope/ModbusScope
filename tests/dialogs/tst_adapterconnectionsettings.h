#include "models/settingsmodel.h"

#include <QObject>

class TestAdapterConnectionSettings : public QObject
{
    Q_OBJECT

private slots:
    void noAdapterShowsLabel();
    void emptySchemaShowsLabel();
    void arrayConnectionsCreatesTabWidget();
    void objectConnectionsCreatesSingleForm();
    void generalSectionHiddenWhenPropertiesEmpty();
    void generalSectionShownWhenNonEmpty();
    void acceptValuesStoresConfigInAdapterData();
};
