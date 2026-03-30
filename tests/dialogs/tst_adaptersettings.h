#ifndef TST_ADAPTERSETTINGS_H
#define TST_ADAPTERSETTINGS_H

#include <QObject>

class TestAdapterSettings : public QObject
{
    Q_OBJECT

private slots:
    void isRenderableArrayWithItems();
    void isRenderableArrayWithoutItems();
    void isRenderableObjectWithProperties();
    void isRenderableObjectWithoutProperties();
    void isRenderableScalarReturnsFalse();

    void arrayPropertyCreatesTabWidget();
    void objectPropertyCreatesSingleForm();
    void addTabUsesPropertyDefaults();
    void acceptValuesStoresConfigInAdapterData();
};

#endif // TST_ADAPTERSETTINGS_H
