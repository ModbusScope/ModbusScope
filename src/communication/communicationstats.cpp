#include "communicationstats.h"

#include "models/communicationstatsmodel.h"
#include "models/graphdatamodel.h"
#include <algorithm>

const uint32_t CommunicationStats::_cUpdateTime = 500;

CommunicationStats::CommunicationStats(GraphDataModel* pGraphDataModel,
                                       CommunicationStatsModel* pStatsModel,
                                       quint32 sampleCalculationSize,
                                       QObject* parent)
    : QObject{ parent },
      _pGraphDataModel(pGraphDataModel),
      _pStatsModel(pStatsModel),
      _sampleCalculationSize(sampleCalculationSize)
{
    connect(this, &CommunicationStats::triggerRunTimeUpdate, _pStatsModel, &CommunicationStatsModel::timeStatsChanged);
}

void CommunicationStats::updateTimingInfo()
{
    QList<double> diffList;
    quint32 timeMedian;

    if (_pGraphDataModel->size() == 0u || _pGraphDataModel->dataMap(0)->size() <= 1)
    {
        timeMedian = 0u;
    }
    else
    {
        auto graphDate = _pGraphDataModel->dataMap(0);
        const quint32 elementCnt = std::min(static_cast<quint32>(graphDate->size()), _sampleCalculationSize);

        auto dataIterator = graphDate->constEnd() - elementCnt;

        for (; dataIterator != graphDate->constEnd() - 1; ++dataIterator)
        {
            double diff = (dataIterator + 1)->mainKey() - dataIterator->mainKey();

            diffList.append(qAbs(diff));
        }

        std::sort(diffList.begin(), diffList.end(), std::less<double>());

        timeMedian = diffList[(elementCnt - 1) / 2];
    }

    _pStatsModel->setMedianPollTime(timeMedian);
}

void CommunicationStats::resetTiming()
{
    _pStatsModel->setStats(0, 0);
    _pStatsModel->setStartTime(QDateTime::currentMSecsSinceEpoch());
    _pStatsModel->setMedianPollTime(0);

    emit triggerRunTimeUpdate();
}

void CommunicationStats::start()
{
    resetTiming();
    _bRunning = true;
    _runtimeTimer.singleShot(_cUpdateTime, this, &CommunicationStats::updateRuntime);
}

void CommunicationStats::stop()
{
    _pStatsModel->setEndTime(QDateTime::currentMSecsSinceEpoch());
    _bRunning = false;
}

void CommunicationStats::incrementCommunicationStats(quint32 successes, quint32 errors)
{
    _pStatsModel->setStats(_pStatsModel->successCount() + successes, _pStatsModel->errorCount() + errors);
}

void CommunicationStats::updateRuntime()
{
    emit triggerRunTimeUpdate();

    if (_bRunning)
    {
        _runtimeTimer.singleShot(_cUpdateTime, this, &CommunicationStats::updateRuntime);
    }
}

void CommunicationStats::updateCommunicationStats(ResultDoubleList resultList)
{
    quint32 error = 0;
    quint32 success = 0;
    for (const auto& result : resultList)
    {
        result.isValid() ? success++ : error++;
    }

    incrementCommunicationStats(success, error);
}
