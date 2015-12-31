
#include "util.h"
#include "guimodel.h"
#include "graphdatamodel.h"
#include "extendedgraphview.h"
#include "communicationmanager.h"
#include "settingsmodel.h"

ExtendedGraphView::ExtendedGraphView(CommunicationManager * pConnMan, GuiModel * pGuiModel, SettingsModel * pSettingsModel, GraphDataModel * pRegisterDataModel, QCustomPlot * pPlot, QObject *parent):
    BasicGraphView(pGuiModel, pRegisterDataModel, pPlot)
{
    Q_UNUSED(parent);

    _pConnMan = pConnMan;
    _pSettingsModel = pSettingsModel;

    //Get difference between UTC and local in milliseconds
    QDateTime local(QDateTime::currentDateTime());
    QDateTime UTC(local.toUTC());
    QDateTime dt(UTC.date(), UTC.time(), Qt::LocalTime);
    _diffWithUtc = dt.secsTo(local) * 1000;

    connect(_pPlot->xAxis, SIGNAL(rangeChanged(QCPRange, QCPRange)), this, SLOT(xAxisRangeChanged(QCPRange, QCPRange)));
}

ExtendedGraphView::~ExtendedGraphView()
{

}

QList<double> ExtendedGraphView::graphTimeData()
{
    return _pPlot->graph(0)->data()->keys();
}

QList<QCPData> ExtendedGraphView::graphData(qint32 index)
{
    return _pPlot->graph(index)->data()->values();
}

void ExtendedGraphView::addData(QList<double> timeData, QList<QList<double> > data)
{
    updateData(&timeData, &data);
}

void ExtendedGraphView::plotResults(QList<bool> successList, QList<double> valueList)
{
    quint64 timeData;
    if (_pSettingsModel->absoluteTimes())
    {
        // Epoch is UTC time, so add the difference between zones in milliseconds
        timeData = QDateTime::currentMSecsSinceEpoch() + _diffWithUtc;
    }
    else
    {
        timeData = QDateTime::currentMSecsSinceEpoch() - _pGuiModel->communicationStartTime();
    }

    QList<double> dataList;

    for (qint32 i = 0; i < valueList.size(); i++)
    {
        if (successList[i])
        {
            // No error, add points
            _pPlot->graph(i)->addData(timeData, valueList[i]);
            dataList.append(valueList[i]);

            _pPlot->graph(i)->setName(QString("(%1) %2").arg(Util::formatDoubleForExport(valueList[i])).arg(_pGraphDataModel->label(i)));
        }
        else
        {
            _pPlot->graph(i)->addData(timeData, 0);
            dataList.append(0);
            _pPlot->graph(i)->setName(QString("(-) %1").arg(_pGraphDataModel->label(i)));
        }
    }

    emit dataAddedToPlot(timeData, dataList);

   rescalePlot();
}

void ExtendedGraphView::clearResults()
{
    for (qint32 i = 0; i < _pPlot->graphCount(); i++)
    {
        _pPlot->graph(i)->clearData();
        _pPlot->graph(i)->setName(QString("(-) %1").arg(_pGraphDataModel->label(i)));
    }

   rescalePlot();
}

void ExtendedGraphView::rescalePlot()
{

    // scale x-axis
    if (_pGuiModel->xAxisScalingMode() == SCALE_AUTO)
    {
        if ((_pPlot->graphCount() != 0) && (_pPlot->graph(0)->data()->keys().size()))
        {
            _pPlot->xAxis->rescale();
        }
        else
        {
            _pPlot->xAxis->setRange(0, 10000);
        }
    }
    else if (_pGuiModel->xAxisScalingMode() == SCALE_SLIDING)
    {
        // sliding window scale routine
        const quint64 slidingInterval = _pGuiModel->xAxisSlidingSec() * 1000;
        if ((_pPlot->graphCount() != 0) && (_pPlot->graph(0)->data()->keys().size()))
        {
            const quint64 lastTime = _pPlot->graph(0)->data()->keys().last();
            if (lastTime > slidingInterval)
            {
                _pPlot->xAxis->setRange(lastTime - slidingInterval, lastTime);
            }
            else
            {
                _pPlot->xAxis->setRange(0, slidingInterval);
            }
        }
        else
        {
            _pPlot->xAxis->setRange(0, slidingInterval);
        }
    }
    else // Manual
    {

    }

    // scale y-axis
    if (_pGuiModel->yAxisScalingMode() == SCALE_AUTO)
    {
        if ((_pPlot->graphCount() != 0) && (_pPlot->graph(0)->data()->keys().size()))
        {
            _pPlot->yAxis->rescale(true);
        }
        else
        {
            _pPlot->yAxis->setRange(0, 10);
        }
    }
    else if (_pGuiModel->yAxisScalingMode() == SCALE_MINMAX)
    {
        // min max scale routine
        _pPlot->yAxis->setRange(_pGuiModel->yAxisMin(), _pGuiModel->yAxisMax());
    }
    else // Manual
    {

    }

    _pPlot->replot();
}

void ExtendedGraphView::updateData(QList<double> *pTimeData, QList<QList<double> > * pDataLists)
{
    bool bFullScale = false;
    const QVector<double> timeData = pTimeData->toVector();

    if (_pPlot->graph(0)->data()->keys().size() > 0)
    {
        if (
        (_pPlot->xAxis->range().lower <= _pPlot->graph(0)->data()->keys().first())
        && (_pPlot->xAxis->range().upper >= _pPlot->graph(0)->data()->keys().last())
        )
        {
            bFullScale = true;
        }
    }
    else
    {
        /* First load of file: always rescale */
        bFullScale = true;
    }

    for (qint32 i = 1; i < pDataLists->size(); i++)
    {
        //Add data to graphs
        QVector<double> graphData = pDataLists->at(i).toVector();
        _pPlot->graph(i - 1)->setData(timeData, graphData);
    }

    if (bFullScale)
    {
        _pPlot->rescaleAxes(true);
    }
    _pPlot->replot();
}

void ExtendedGraphView::xAxisRangeChanged(const QCPRange &newRange, const QCPRange &oldRange)
{
    QCPRange range = newRange;

    if (newRange.upper <= 0)
    {
        range.upper = oldRange.upper;
    }

    if (newRange.lower <= 0)
    {
        range.lower = 0;
    }

    _pPlot->xAxis->setRange(range);
}
