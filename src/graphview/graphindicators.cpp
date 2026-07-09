
#include "graphindicators.h"

#include "graphview/graphview.h"
#include "models/graphdatamodel.h"

GraphIndicators::GraphIndicators(GraphDataModel* pGraphDataModel, ScopePlot* pPlot, QObject* parent)
    : QObject(parent), _pGraphDataModel(pGraphDataModel), _pPlot(pPlot)
{
    connect(_pPlot->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this,
            QOverload<const QCPRange&>::of(&GraphIndicators::setTracerPosition));
    connect(_pPlot->yAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this,
            QOverload<>::of(&GraphIndicators::setTracerPosition));
    connect(_pPlot->yAxis2, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this,
            QOverload<>::of(&GraphIndicators::setTracerPosition));

    connect(_pGraphDataModel, &GraphDataModel::colorChanged, this, &GraphIndicators::updateColor);
    connect(_pGraphDataModel, &GraphDataModel::valueAxisChanged, this, &GraphIndicators::updateValueAxis);
}

GraphIndicators::~GraphIndicators()
{
    clear();
}

void GraphIndicators::clear()
{
    while (!_valueTracers.isEmpty())
    {
        _pPlot->removeItem(_valueTracers.last());
        _valueTracers.removeLast();

        _pPlot->removeItem(_axisValueTracers.last());
        _axisValueTracers.removeLast();
    }
}

/*!
 * \brief Adds a pair of tracers for the given graph to track its value on the axis.
 * \param graphIdx Graph index in the full graph list.
 * \param pGraph Pointer to the QCPGraph to attach the tracer to.
 */
void GraphIndicators::add(GraphIdx graphIdx, QCPGraph* pGraph)
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
    for (qint32 idx = 0; idx < _axisValueTracers.size(); idx++)
    {
        _axisValueTracers.at(idx)->setVisible(determineVisibility(ActiveIdx(idx)));
    }
}

/*!
 * \brief Returns whether the axis tracer for the given active graph should be visible.
 * \param activeIdx Sequential slot index of the active graph.
 * \return True if the tracer is in range and the graph is visible with data; false otherwise.
 */
bool GraphIndicators::determineVisibility(ActiveIdx activeIdx)
{
    const GraphIdx graphIdx = _pGraphDataModel->convertToGraphIndex(activeIdx);

    auto pPos = _axisValueTracers.at(activeIdx.v)->position;

    bool bVisibility = _pGraphDataModel->isVisible(graphIdx) && !_pGraphDataModel->dataSeries(graphIdx)->isEmpty() &&
                       (pPos->value() >= pPos->valueAxis()->range().lower) &&
                       (pPos->value() <= pPos->valueAxis()->range().upper);

    return bVisibility;
}

/*!
 * \brief Moves the tracer for the given graph to the top drawing layer.
 * \param graphIdx Graph index in the full graph list.
 */
void GraphIndicators::setFrontGraph(GraphIdx graphIdx)
{
    const ActiveIdx activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

    if (activeIdx.isValid())
    {
        _axisValueTracers.at(activeIdx.v)->setLayer("topAxes");
        _pPlot->replot(QCustomPlot::rpRefreshHint);
    }
}

void GraphIndicators::setTracerPosition()
{
    setTracerPosition(_pPlot->xAxis->range());
}

void GraphIndicators::setTracerPosition(const QCPRange& newRange)
{
    for (qint32 idx = 0; idx < _valueTracers.size(); idx++)
    {
        const GraphIdx graphIdx = _pGraphDataModel->convertToGraphIndex(ActiveIdx(idx));
        const double key =
          _pGraphDataModel->valueAxis(graphIdx) == GraphData::VALUE_AXIS_PRIMARY ? newRange.lower : newRange.upper;
        const double axisRectCoord = _pGraphDataModel->valueAxis(graphIdx) == GraphData::VALUE_AXIS_PRIMARY ? 0 : 1;

        /* Use hidden tracer to get correct value */
        _valueTracers.at(idx)->setGraphKey(key);
        _valueTracers.at(idx)->updatePosition();

        /* Set key to Y-axis intersection and value to interpolated value */
        _axisValueTracers.at(idx)->position->setCoords(axisRectCoord, _valueTracers.at(idx)->position->value());
    }

    updateVisibility();

    _pPlot->replot(QCustomPlot::rpRefreshHint);
}

/*!
 * \brief Updates the tracer pen color to match the current graph color.
 * \param graphIdx Graph index in the full graph list.
 */
void GraphIndicators::updateColor(GraphIdx graphIdx)
{
    const ActiveIdx activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

    if (activeIdx.isValid())
    {
        auto pen = _axisValueTracers.at(activeIdx.v)->pen();
        pen.setColor(_pGraphDataModel->color(graphIdx));
        _axisValueTracers.at(activeIdx.v)->setPen(pen);
    }
}

/*!
 * \brief Re-configures the tracer axis and repositions it after a value-axis change.
 * \param graphIdx Graph index in the full graph list.
 */
void GraphIndicators::updateValueAxis(GraphIdx graphIdx)
{
    configureValueAxis(graphIdx);

    setTracerPosition(_pPlot->xAxis->range());
}

/*!
 * \brief Binds the tracer position to the correct primary or secondary value axis.
 * \param graphIdx Graph index in the full graph list.
 */
void GraphIndicators::configureValueAxis(GraphIdx graphIdx)
{
    const ActiveIdx activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

    if (activeIdx.isValid())
    {
        auto valueAxis =
          _pGraphDataModel->valueAxis(graphIdx) == GraphData::VALUE_AXIS_PRIMARY ? _pPlot->yAxis : _pPlot->yAxis2;
        _axisValueTracers.at(activeIdx.v)->position->setAxisRect(valueAxis->axisRect());
        _axisValueTracers.at(activeIdx.v)->position->setAxes(nullptr, valueAxis);
    }
}
