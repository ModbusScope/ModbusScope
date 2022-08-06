
#include "graphindicators.h"

#include "graphdatamodel.h"
#include "graphview.h"

GraphIndicators::GraphIndicators(GraphDataModel * pGraphDataModel, ScopePlot* pPlot, QObject *parent) :
    QObject(parent),
    _pGraphDataModel(pGraphDataModel),
    _pPlot(pPlot),
    _pGraph(nullptr)
{

    /* TODO: Add visibility */


    /* Handle color change */
    /* Handle axis changed */

    /* rangeChanged */
    connect(_pPlot->xAxis, QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged), this, &GraphIndicators::axisRangeChanged);
}

GraphIndicators::~GraphIndicators()
{

}

void GraphIndicators::clear()
{
    while(!_axisTracerList.isEmpty())
    {
        _pPlot->removeItem(_axisTracerList.last());
        _axisTracerList.removeLast();
    }
}

void GraphIndicators::add(QCPGraph* pGraph)
{    
    auto tracer = new QCPItemTracer(_pPlot);
    tracer->setStyle(QCPItemTracer::tsCircle);
    tracer->setSize(8);
    tracer->setInterpolating(true);
    tracer->setLayer(QLatin1String("axes"));
    tracer->setClipToAxisRect(false);

    tracer->setGraph(pGraph);
    tracer->setGraphKey(_pPlot->xAxis->range().lower);

    _axisTracerList.append(tracer);
}

void GraphIndicators::axisRangeChanged(const QCPRange &newRange)
{
    double newLower = newRange.lower;
    //double newUpper = newRange.upper;

    QList<QCPGraph*> const graphList = _pPlot->xAxis->graphs();

    if (graphList.size() > 0 && !graphList[0]->data()->isEmpty())
    {
        setTracerPosition(newLower);
    }
}

void GraphIndicators::setTracerPosition(double key)
{
    for (uint32_t idx = 0; idx < _axisTracerList.size(); idx++)
    {
        _axisTracerList[idx]->setGraphKey(key);
    }
}
