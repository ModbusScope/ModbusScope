#ifndef TST_DATAPOINTDESCRIPTIONCACHE_H
#define TST_DATAPOINTDESCRIPTIONCACHE_H

#include "util/expressionreferences.h"

#include <QObject>

class DataPointDescriptionCache;
class DataPointService;
class MockAdapterHub;
class MockDataPointManager;
class SettingsModel;

class TestDataPointDescriptionCache : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void missSendsRequest();
    void repeatedLookupWhilePendingSendsOnce();
    void validResultIsCached();
    void invalidResultNeverRetried();
    void failedRetriedOnlyAfterInterval();
    void unavailableWithoutRequest();
    void unavailableRetriedAfterAdaptersChanged();
    void failedRetriedAfterAdaptersChanged();
    void pendingReplyAppliedAfterAdaptersChanged();
    void otherTicketsIgnored();
    void adapterChangeRequestsNewDescription();
    void replyFromPreviousAdapterIgnored();

private:
    static ExpressionReferences::Reference reference(const QString& text);

    SettingsModel* _pSettingsModel{ nullptr };
    MockAdapterHub* _pMockHub{ nullptr };
    MockDataPointManager* _pManager{ nullptr };
    DataPointService* _pService{ nullptr };
    DataPointDescriptionCache* _pCache{ nullptr };
};

#endif // TST_DATAPOINTDESCRIPTIONCACHE_H
