#ifndef TST_ADAPTERDISCOVERY_H
#define TST_ADAPTERDISCOVERY_H

#include <QObject>

class TestAdapterDiscovery : public QObject
{
    Q_OBJECT
private slots:
    void discoverEmpty();
    void discoverSingleAdapter();
    void discoverMultipleAdapters();
    void ignoreNonAdapterFiles();
    void ignoreEmptyIdAdapter();
};

#endif // TST_ADAPTERDISCOVERY_H
