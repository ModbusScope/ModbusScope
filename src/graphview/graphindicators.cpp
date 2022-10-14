
#include "graphindicators.h"

#include "graphdatamodel.h"
#include "graphview.h"

GraphIndicators::GraphIndicators(GraphDataModel * pGraphDataModel, ScopePlot* pPlot, QObject *parent) :
    QObject(parent),
    _pGraphDataModel(pGraphDataModel),
    _pPlot(pPlot)
{
    connect(_pPlot->xAxis, QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged), this, QOverload<const QCPRange &>::of(&GraphIndicators::setTracerPosition));
    connect(_pPlot->yAxis, QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged), this, QOverload<>::of(&GraphIndicators::setTracerPosition));
    connect(_pPlot->yAxis2, QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged), this, QOverload<>::of(&GraphIndicators::setTracerPosition));

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

    setTracerPosition();
}

void GraphIndicators::updateIndicatorVisibility()
{
    setTracerPosition();
}

void GraphIndicators::updateVisibility()
{
    for (uint32_t idx = 0; idx < _axisValueTracers.size(); idx++)
    {
        _axisValueTracers[idx]->setVisible(determineVisibility(idx));
    }
}

bool GraphIndicators::determineVisibility(uint32_t activeIdx)
{
    const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(activeIdx);

    auto pPos = _axisValueTracers[activeIdx]->position;

    bool bVisibility = _pGraphDataModel->isVisible(graphIdx)
                        && !_pGraphDataModel->dataMap(graphIdx)->isEmpty()
                        && (pPos->value() >= pPos->valueAxis()->range().lower)
                        && (pPos->value() <= pPos->valueAxis()->range().upper);

    return bVisibility;
}

void GraphIndicators::setFrontGraph(quint32 graphIdx)
{
    const qint32 activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

    if (activeIdx != -1)
    {
        _axisValueTracers[activeIdx]->setLayer("topAxes");
        _pPlot->replot(QCustomPlot::rpRefreshHint);
    }
}

void GraphIndicators::setTracerPosition()
{
    setTracerPosition(_pPlot->xAxis->range());
}

void GraphIndicators::setTracerPosition(const QCPRange &newRange)
{
    for (uint32_t idx = 0; idx < _valueTracers.size(); idx++)
    {
        const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(idx);
        const double key = _pGraphDataModel->valueAxis(graphIdx) == GraphData::VALUE_AXIS_PRIMARY ? newRange.lower: newRange.upper;
        const double axisRectCoord = _pGraphDataModel->valueAxis(graphIdx) == GraphData::VALUE_AXIS_PRIMARY ? 0: 1;

        /* Use hidden tracer to get correct value */
        _valueTracers[idx]->setGraphKey(key);
        _valueTracers[idx]->updatePosition();

        /* Set key to Y-axis intersection and value to interpolated value */
        _axisValueTracers[idx]->position->setCoords(axisRectCoord, _valueTracers[idx]->position->value());
    }

    updateVisibility();

    _pPlot->replot(QCustomPlot::rpRefreshHint);
}

void GraphIndicators::updateColor(quint32 graphIdx)
{
    const qint32 activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

    if (activeIdx != -1)
    {
        auto pen = _axisValueTracers[activeIdx]->pen();
        pen.setColor(_pGraphDataModel->color(graphIdx));
        _axisValueTracers[activeIdx]->setPen(pen);
    }
}

void GraphIndicators::updateValueAxis(quint32 graphIdx)
{
    configureValueAxis(graphIdx);

    setTracerPosition(_pPlot->xAxis->range());
}

void GraphIndicators::configureValueAxis(quint32 graphIdx)
{
    const qint32 activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

    if (activeIdx != -1)
    {
        auto valueAxis = _pGraphDataModel->valueAxis(graphIdx) == GraphData::VALUE_AXIS_PRIMARY ? _pPlot->yAxis: _pPlot->yAxis2;
        _axisValueTracers[activeIdx]->position->setAxisRect(valueAxis->axisRect());
        _axisValueTracers[activeIdx]->position->setAxes(nullptr, valueAxis);
    }
}
