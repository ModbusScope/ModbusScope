
#include "graphindicators.h"

#include "graphdatamodel.h"
#include "graphview.h"

GraphIndicators::GraphIndicators(GraphDataModel * pGraphDataModel, ScopePlot* pPlot, QObject *parent) :
    QObject(parent),
    _pGraphDataModel(pGraphDataModel),
    _pPlot(pPlot),
    _pGraph(nullptr)
{

    /* Handle axis changed */

    connect(_pPlot->xAxis, QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged), this, &GraphIndicators::axisRangeChanged);
    connect(_pGraphDataModel, &GraphDataModel::colorChanged, this, &GraphIndicators::updateColor);
}

GraphIndicators::~GraphIndicators()
{

}

void GraphIndicators::clear()
{
    while(!_valueTracers.isEmpty())
    {
        _pPlot->removeItem(_valueTracers.last());
        _valueTracers.removeLast();

        _pPlot->removeItem(_axisValueTracers.last());
        _axisValueTracers.removeLast();
    }
}

void GraphIndicators::add(QCPGraph* pGraph)
{
    _pGraph = pGraph;

    /* Hidden tracer to get correct interpolated value */
    auto valueTracer = new QCPItemTracer(_pPlot);
    valueTracer->setVisible(false);
    valueTracer->setInterpolating(true);
    valueTracer->setGraph(pGraph);
    valueTracer->setGraphKey(_pPlot->xAxis->range().lower);

    _valueTracers.append(valueTracer);

    /*
     * Actual tracer: required because the hidden tracer is connected to graph
     * When connected to a graph, the key will be limited to a valid range,
     * which might not the correct position.
     */
    auto axisValueTracer = new QCPItemTracer(_pPlot);
    axisValueTracer->setStyle(QCPItemTracer::tsCircle);
    axisValueTracer->setSize(4u);

    auto pen = axisValueTracer->pen();
    pen.setWidth(axisValueTracer->size());
    pen.setColor(_pGraph->pen().color());
    axisValueTracer->setPen(pen);

    axisValueTracer->setVisible(true);
    axisValueTracer->position->setTypeX(QCPItemPosition::ptAxisRectRatio);
    axisValueTracer->position->setTypeY(QCPItemPosition::ptPlotCoords);
    axisValueTracer->position->setAxisRect(_pPlot->yAxis->axisRect());
    axisValueTracer->position->setAxes(nullptr, _pPlot->yAxis);
    axisValueTracer->position->setCoords(0, 0);

    axisValueTracer->setLayer(QLatin1String("axes"));
    axisValueTracer->setClipToAxisRect(false);

    _axisValueTracers.append(axisValueTracer);
}

void GraphIndicators::updateVisibility()
{
    for (uint32_t idx = 0; idx < _valueTracers.size(); idx++)
    {
        const bool bVisibility = _pGraphDataModel->isVisible(idx);
        _axisValueTracers[idx]->setVisible(bVisibility);
    }
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
    for (uint32_t idx = 0; idx < _valueTracers.size(); idx++)
    {
        /* Use hidden tracer to get correct value */
        _valueTracers[idx]->setGraphKey(key);
        _valueTracers[idx]->updatePosition();

        /* Set key to Y-axis intersection and value to interpolated value */
        _axisValueTracers[idx]->position->setCoords(0, _valueTracers[idx]->position->value());
    }
}

void GraphIndicators::updateColor(quint32 graphIdx)
{
    Q_UNUSED(graphIdx);
    if (graphIdx < _axisValueTracers.size())
    {
        auto pen = _axisValueTracers[graphIdx]->pen();
        pen.setColor(_pGraphDataModel->color(graphIdx));
        _axisValueTracers[graphIdx]->setPen(pen);
    }
}
