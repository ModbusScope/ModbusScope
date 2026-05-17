#ifndef COMMUNICATIONSTATS_H
#define COMMUNICATIONSTATS_H

#include "util/result.h"
#include <QObject>
#include <QTimer>

class GraphDataModel;
class CommunicationStatsModel;

class CommunicationStats : public QObject
{
    Q_OBJECT
public:
    explicit CommunicationStats(GraphDataModel* pGraphDataModel,
                                CommunicationStatsModel* pStatsModel,
                                quint32 sampleCalculationSize = 50,
                                QObject* parent = nullptr);

    void resetTiming();
    void start();
    void stop();

    void incrementCommunicationStats(quint32 successCount, quint32 errors);

signals:
    void triggerRunTimeUpdate();

public slots:
    void updateTimingInfo();
    void updateCommunicationStats(ResultDoubleList resultList);

private slots:
    void updateRuntime();

private:
    GraphDataModel* _pGraphDataModel;
    CommunicationStatsModel* _pStatsModel;

    bool _bRunning{ false };

    QTimer _runtimeTimer;

    quint32 _sampleCalculationSize = 50;

    static const uint32_t _cUpdateTime;
};

#endif // COMMUNICATIONSTATS_H
