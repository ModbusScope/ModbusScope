#include "communicationstats.h"

#include "graphdatamodel.h"
#include <algorithm>

const uint32_t CommunicationStats::_cUpdateTime = 500;

CommunicationStats::CommunicationStats(GraphDataModel* pGraphDataModel, quint32 sampleCalculationSize, QObject *parent)
    : QObject{parent}, _pGraphDataModel (pGraphDataModel), _sampleCalculationSize(sampleCalculationSize)
{
    connect(this, &CommunicationStats::triggerRunTimeUpdate, _pGraphDataModel, &GraphDataModel::communicationTimeStatsChanged);
}

void CommunicationStats::updateTimingInfo()
{
    QList<double> diffList;

    if (_pGraphDataModel->size() == 0u)
    {
        _timeMedian = 0u;
        return;
    }

    if (_pGraphDataModel->dataMap(0)->size() <= 1)
    {
        _timeMedian = 0u;
        return;
    }

    auto graphDate = _pGraphDataModel->dataMap(0);
    const quint32 elementCnt = std::min(static_cast<quint32>(graphDate->size()), _sampleCalculationSize);

    auto dataIterator = graphDate->constEnd() - elementCnt;

    for (; dataIterator != graphDate->constEnd() - 1; ++dataIterator)
    {
        double diff = (dataIterator + 1)->mainKey() - dataIterator->mainKey();

        diffList.append(qAbs(diff));
    }

    std::sort(diffList.begin(), diffList.end(), std::less<double>());

    _timeMedian = diffList[(elementCnt - 1) / 2];

    _pGraphDataModel->setMedianPollTime(_timeMedian);
}

quint32 CommunicationStats::pollTimeMedian()
{
    return _timeMedian;
}

void CommunicationStats::resetTiming()
{
    _pGraphDataModel->setCommunicationStats(0, 0);
    _pGraphDataModel->setCommunicationStartTime(QDateTime::currentMSecsSinceEpoch());
    _timeMedian = 0;

    emit triggerRunTimeUpdate();
}

void CommunicationStats::start(uint32_t pollTime)
{
    resetTiming();
    _timeMedian = pollTime;
    _bRunning = true;
    _runtimeTimer.singleShot(_cUpdateTime, this, &CommunicationStats::updateRuntime);
}

void CommunicationStats::stop()
{
    _pGraphDataModel->setCommunicationEndTime(QDateTime::currentMSecsSinceEpoch());
    _bRunning = false;
}

void CommunicationStats::incrementCommunicationStats(quint32 successes, quint32 errors)
{
    _pGraphDataModel->setCommunicationStats(_pGraphDataModel->communicationSuccessCount() + successes, _pGraphDataModel->communicationErrorCount() + errors);
}

void CommunicationStats::updateRuntime()
{
    emit triggerRunTimeUpdate();

    // restart timer
    if (_bRunning)
    {
        _runtimeTimer.singleShot(_cUpdateTime, this, &CommunicationStats::updateRuntime);
    }
}

void CommunicationStats::updateCommunicationStats(ResultDoubleList resultList)
{
    quint32 error = 0;
    quint32 success = 0;
    for(const auto &result: resultList)
    {
        result.isValid() ? success++ : error++;
    }

    incrementCommunicationStats(success, error);
}
