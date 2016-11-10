
#include "util.h"
#include "guimodel.h"
#include "graphdatamodel.h"
#include "extendedgraphview.h"
#include "communicationmanager.h"
#include "settingsmodel.h"

ExtendedGraphView::ExtendedGraphView(CommunicationManager * pConnMan, GuiModel * pGuiModel, SettingsModel * pSettingsModel, GraphDataModel * pRegisterDataModel, MyQCustomPlot *pPlot, QObject *parent):
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

void ExtendedGraphView::addData(QList<double> timeData, QList<QList<double> > data)
{
    updateData(&timeData, &data);
}

void ExtendedGraphView::plotResults(QList<bool> successList, QList<double> valueList)
{
    /* QList correspond with activeGraphList */

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

    for (qint32 i = 1; i < pDataLists->size(); i++)
    {
        //Add data to graphs
        QVector<double> graphData = pDataLists->at(i).toVector();
        _pPlot->graph(i - 1)->setData(timeData, graphData);

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
