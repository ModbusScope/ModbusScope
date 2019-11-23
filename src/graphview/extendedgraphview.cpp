
#include "util.h"
#include "guimodel.h"
#include "graphdatamodel.h"
#include "extendedgraphview.h"
#include "myqcpaxis.h"
#include "communicationmanager.h"
#include "settingsmodel.h"

ExtendedGraphView::ExtendedGraphView(CommunicationManager * pConnMan, GuiModel * pGuiModel, SettingsModel * pSettingsModel, GraphDataModel * pRegisterDataModel, NoteModel * pNoteModel, MyQCustomPlot *pPlot, QObject *parent):
    BasicGraphView(pGuiModel, pRegisterDataModel, pNoteModel, pPlot)
{
    Q_UNUSED(parent);

    _pConnMan = pConnMan;
    _pSettingsModel = pSettingsModel;

    connect(_pPlot->xAxis, SIGNAL(rangeChanged(QCPRange, QCPRange)), this, SLOT(xAxisRangeChanged(QCPRange, QCPRange)));
}

ExtendedGraphView::~ExtendedGraphView()
{

}

void ExtendedGraphView::addData(QList<double> timeData, QList<QList<double> > data)
{
    _pGuiModel->setxAxisScale(BasicGraphView::SCALE_AUTO);
    _pGuiModel->setyAxisScale(BasicGraphView::SCALE_AUTO);

    updateData(&timeData, &data);
}

void ExtendedGraphView::showGraph(quint32 graphIdx)
{
    if (_pGraphDataModel->isActive(graphIdx))
    {
        const bool bShow = _pGraphDataModel->isVisible(graphIdx);

        const quint32 activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

        _pPlot->graph(activeIdx)->setVisible(bShow);

        rescalePlot();
    }
}

void ExtendedGraphView::rescalePlot()
{

    // scale x-axis
    if (_pGuiModel->xAxisScalingMode() == SCALE_AUTO)
    {
        if ((_pPlot->graphCount() != 0) && (graphDataSize() != 0))
        {
            _pPlot->xAxis->rescale(true);
        }
        else
        {
            _pPlot->xAxis->setRange(0, 10000);
        }
    }
    else if (_pGuiModel->xAxisScalingMode() == SCALE_SLIDING)
    {
        // sliding window scale routine
        const quint64 slidingInterval = static_cast<quint64>(_pGuiModel->xAxisSlidingSec()) * 1000;
        if ((_pPlot->graphCount() != 0) && (graphDataSize() != 0))
        {
            auto lastDataIt = _pPlot->graph(0)->data()->constEnd();
            lastDataIt--; /* Point to last existing item */

            const quint64 lastTime = (quint64)lastDataIt->key;
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
        if ((_pPlot->graphCount() != 0) && (graphDataSize()))
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
    else if (_pGuiModel->yAxisScalingMode() == SCALE_WINDOW_AUTO)
    {
        (dynamic_cast<MyQCPAxis *>(_pPlot->yAxis))->rescaleValue(_pPlot->xAxis->range());
    }
    else // Manual
    {

    }

    _pPlot->replot();
}

void ExtendedGraphView::plotResults(QList<bool> successList, QList<double> valueList)
{
    /* QList correspond with activeGraphList */

    quint64 timeData;
    if (_pSettingsModel->absoluteTimes())
    {
        // Epoch is in UTC time
        timeData = QDateTime::currentMSecsSinceEpoch();
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
        }
        else
        {
            _pPlot->graph(i)->addData(timeData, 0);
            dataList.append(0);
        }
    }

    emit dataAddedToPlot(timeData, dataList);

   rescalePlot();
}

void ExtendedGraphView::clearResults()
{
    for (qint32 i = 0; i < _pPlot->graphCount(); i++)
    {
        _pPlot->graph(i)->data()->clear();
        _pPlot->graph(i)->setName(QString("(-) %1").arg(_pGraphDataModel->label(i)));
    }

   rescalePlot();
}

void ExtendedGraphView::updateData(QList<double> *pTimeData, QList<QList<double> > * pDataLists)
{
    quint64 totalPoints = 0;
    const QVector<double> timeData = pTimeData->toVector();

    for (qint32 i = 0; i < pDataLists->size(); i++)
    {
        //Add data to graphs
        QVector<double> graphData = pDataLists->at(i).toVector();
        _pPlot->graph(i)->setData(timeData, graphData);

        totalPoints += graphData.size();
    }

    // Check if optimizations are needed
    if (totalPoints > _cOptimizeThreshold)
    {
        _pGuiModel->setHighlightSamples(false);

        // Set width to 1
        for (qint32 i = 0; i <  _pPlot->graphCount(); i++)
        {
             _pPlot->graph(i)->pen().setWidth(1);
        }

        // Disable anti aliasing
        _pPlot->setNotAntialiasedElements(QCP::aeAll);
    }

    _pPlot->rescaleAxes(true);
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
