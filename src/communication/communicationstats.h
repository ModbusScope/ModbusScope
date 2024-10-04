#ifndef COMMUNICATIONSTATS_H
#define COMMUNICATIONSTATS_H

#include <QObject>
#include <QTimer>
#include "result.h"

class GraphDataModel;

class CommunicationStats : public QObject
{
    Q_OBJECT
public:
    explicit CommunicationStats(GraphDataModel* pGraphDataModel, quint32 sampleCalculationSize = 50, QObject *parent = nullptr);

    quint32 pollTimeMedian();

    void resetTiming();
    void start(uint32_t pollTime);
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

    bool _bRunning{false};

    QTimer _runtimeTimer;

    quint32 _timeMedian = 0;
    quint32 _sampleCalculationSize = 50;

    static const uint32_t _cUpdateTime;
};

#endif // COMMUNICATIONSTATS_H
