
#include "graphmarkers.h"
#include "graphdatamodel.h"
#include "guimodel.h"
#include "graphview.h"

GraphMarkers::GraphMarkers(GraphDataModel * pGraphDataModel, GuiModel* pGuiModel, ScopePlot* pPlot, QObject *parent) :
    QObject(parent),
    _pGuiModel(pGuiModel),
    _pGraphDataModel(pGraphDataModel),
    _pPlot(pPlot),
    _pGraphview(dynamic_cast<GraphView*>(parent))
{
    QPen markerPen;
    markerPen.setWidth(1);

    markerPen.setColor(QColor(Qt::green));
    _pStartMarker = new QCPItemStraightLine(_pPlot);
    _pStartMarker->setVisible(false);
    _pStartMarker->setPen(markerPen);

    markerPen.setColor(QColor(Qt::red));
    _pEndMarker = new QCPItemStraightLine(_pPlot);
    _pEndMarker->setVisible(false);
    _pEndMarker->setPen(markerPen);

    connect(_pGuiModel, &GuiModel::markerStateChanged, this, &GraphMarkers::updateMarkersVisibility);
    connect(_pGuiModel, &GuiModel::startMarkerPosChanged, this, &GraphMarkers::setStartMarker);
    connect(_pGuiModel, &GuiModel::endMarkerPosChanged, this, &GraphMarkers::setEndMarker);

    connect(_pGraphDataModel, &GraphDataModel::valueAxisChanged, this, &GraphMarkers::updateValueAxis);
}

GraphMarkers::~GraphMarkers()
{

}

void GraphMarkers::clearTracers()
{
    while(!_startTracerList.isEmpty())
    {
        _pPlot->removeItem(_startTracerList.last());
        _startTracerList.removeLast();
    }

    while(!_endTracerList.isEmpty())
    {
        _pPlot->removeItem(_endTracerList.last());
        _endTracerList.removeLast();
    }
}

void GraphMarkers::addTracer(QCPGraph* pGraph)
{
    auto startTracer = createTracer(pGraph);
    startTracer->setGraphKey(_pGuiModel->startMarkerPos());
    _startTracerList.append(startTracer);

    auto endTracer = createTracer(pGraph);
    endTracer->setGraphKey(_pGuiModel->endMarkerPos());
    _endTracerList.append(endTracer);

    updateTracersVisibility();
}

void GraphMarkers::updateTracersVisibility()
{
    setTracerVisibility(_startTracerList, _pStartMarker->visible());
    setTracerVisibility(_endTracerList, _pEndMarker->visible());
}

void GraphMarkers::updateMarkersVisibility()
{
    if (_pGuiModel->markerState() == false)
    {
        _pStartMarker->setVisible(false);
        _pEndMarker->setVisible(false);
        setTracerVisibility(_startTracerList, false);
        setTracerVisibility(_endTracerList, false);

        _pPlot->replot();
    }
}

void GraphMarkers::setStartMarker()
{
    const double pos = _pGuiModel->startMarkerPos();

    _pStartMarker->setVisible(true);
    _pStartMarker->point1->setCoords(pos, 0);
    _pStartMarker->point2->setCoords(pos, 1);

    setTracerVisibility(_startTracerList, true);
    setTracerPosition(_startTracerList, pos);

    _pPlot->replot();
}

void GraphMarkers::setEndMarker()
{
    const double pos = _pGuiModel->endMarkerPos();

    _pEndMarker->setVisible(true);
    _pEndMarker->point1->setCoords(pos, 0);
    _pEndMarker->point2->setCoords(pos, 1);

    setTracerVisibility(_endTracerList, true);
    setTracerPosition(_endTracerList, pos);

    _pPlot->replot();
}

void GraphMarkers::updateValueAxis(quint32 graphIdx)
{
    const qint32 activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

    if (activeIdx != -1)
    {
        auto valueAxis = _pGraphDataModel->valueAxis(graphIdx) == GraphData::VALUE_AXIS_PRIMARY ? _pPlot->yAxis: _pPlot->yAxis2;
        _startTracerList[activeIdx]->position->setAxes(_pPlot->xAxis, valueAxis);
        _endTracerList[activeIdx]->position->setAxes(_pPlot->xAxis, valueAxis);
    }
}

void GraphMarkers::setTracerVisibility(QList<QCPItemTracer *> &tracerList, bool bMarkerVisibility)
{
    for (uint32_t activeIdx = 0; activeIdx < tracerList.size(); activeIdx++)
    {
        const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(activeIdx);

        const bool bVisility = bMarkerVisibility && _pGraphDataModel->isVisible(graphIdx);
        tracerList[activeIdx]->setVisible(bVisility);
    }
}

void GraphMarkers::setTracerPosition(QList<QCPItemTracer *> &tracerList, double pos)
{
    for (auto tracer : tracerList)
    {
        tracer->setGraphKey(pos);
    }
}

QCPItemTracer* GraphMarkers::createTracer(QCPGraph* pGraph)
{
    auto tracer = new QCPItemTracer(_pPlot);
    tracer->setVisible(false);
    tracer->setStyle(QCPItemTracer::tsSquare);
    tracer->setSize(8);
    tracer->setInterpolating(true);
    tracer->setLayer("topAxes");
    tracer->setGraph(pGraph);

    return tracer;
}

