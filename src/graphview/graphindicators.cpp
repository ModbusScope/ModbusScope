
#include "graphindicators.h"

#include "graphdatamodel.h"
#include "graphview.h"

GraphIndicators::GraphIndicators(GraphDataModel * pGraphDataModel, ScopePlot* pPlot, QObject *parent) :
    QObject(parent),
    _pGraphDataModel(pGraphDataModel),
    _pPlot(pPlot)
{
    connect(_pPlot->xAxis, QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged), this, &GraphIndicators::setTracerPosition);
    connect(_pGraphDataModel, &GraphDataModel::colorChanged, this, &GraphIndicators::updateColor);
    connect(_pGraphDataModel, &GraphDataModel::valueAxisChanged, this, &GraphIndicators::updateValueAxis);
}

GraphIndicators::~GraphIndicators()
{
    clear();
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

void GraphIndicators::add(quint32 graphIdx, QCPGraph* pGraph)
{
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
    _axisValueTracers.append(axisValueTracer);

    axisValueTracer->setStyle(QCPItemTracer::tsCircle);
    axisValueTracer->setSize(4u);

    auto pen = axisValueTracer->pen();
    pen.setWidth(axisValueTracer->size());
    pen.setColor(_pGraphDataModel->color(graphIdx));
    axisValueTracer->setPen(pen);

    axisValueTracer->setVisible(true);
    axisValueTracer->position->setTypeX(QCPItemPosition::ptAxisRectRatio);
    axisValueTracer->position->setTypeY(QCPItemPosition::ptPlotCoords);

    configureValueAxis(graphIdx);

    /* Make sure tracer is fully visible */
    axisValueTracer->setLayer(QLatin1String("axes"));
    axisValueTracer->setClipToAxisRect(false);

    setTracerPosition(_pPlot->xAxis->range());
}

void GraphIndicators::updateVisibility()
{
    for (uint32_t idx = 0; idx < _valueTracers.size(); idx++)
    {
        const bool bVisibility = _pGraphDataModel->isVisible(idx);
        _axisValueTracers[idx]->setVisible(bVisibility);
    }
}

void GraphIndicators::setTracerPosition(const QCPRange &newRange)
{
    for (uint32_t idx = 0; idx < _valueTracers.size(); idx++)
    {
        const double key = _pGraphDataModel->valueAxis(idx) == GraphData::VALUE_AXIS_PRIMARY ? newRange.lower: newRange.upper;
        const double axisRectCoord = _pGraphDataModel->valueAxis(idx) == GraphData::VALUE_AXIS_PRIMARY ? 0: 1;

        /* Use hidden tracer to get correct value */
        _valueTracers[idx]->setGraphKey(key);
        _valueTracers[idx]->updatePosition();

        /* Set key to Y-axis intersection and value to interpolated value */
        _axisValueTracers[idx]->position->setCoords(axisRectCoord, _valueTracers[idx]->position->value());
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

void GraphIndicators::updateValueAxis(quint32 graphIdx)
{
    configureValueAxis(graphIdx);

    setTracerPosition(_pPlot->xAxis->range());
}

void GraphIndicators::configureValueAxis(quint32 graphIdx)
{
    auto valueAxis = _pGraphDataModel->valueAxis(graphIdx) == GraphData::VALUE_AXIS_PRIMARY ? _pPlot->yAxis: _pPlot->yAxis2;
    _axisValueTracers[graphIdx]->position->setAxisRect(valueAxis->axisRect());
    _axisValueTracers[graphIdx]->position->setAxes(nullptr, valueAxis);
}
