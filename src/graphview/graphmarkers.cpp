
#include "graphmarkers.h"

#include "guimodel.h"
#include "graphview.h"

GraphMarkers::GraphMarkers(GuiModel* pGuiModel, ScopePlot* pPlot, QObject *parent) :
    QObject(parent),
    _pGuiModel(pGuiModel),
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
}

void GraphMarkers::addTracer(QCPGraph* pGraph)
{
    auto tracer = createTracer(pGraph);
    tracer->setGraphKey(_pGuiModel->startMarkerPos());
    _startTracerList.append(tracer);

    tracer = createTracer(pGraph);
    tracer->setGraphKey(_pGuiModel->endMarkerPos());
    _endTracerList.append(tracer);
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

void GraphMarkers::setTracerVisibility(QList<QCPItemTracer *> &tracerList, bool bMarkerVisibility)
{
    for (auto tracer : tracerList)
    {
        const bool bVisility = bMarkerVisibility && tracer->graph()->visible();
        tracer->setVisible(bVisility);
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
    tracer->setVisible(_pGuiModel->markerState() && pGraph->visible());
    tracer->setStyle(QCPItemTracer::tsSquare);
    tracer->setSize(8);
    tracer->setInterpolating(true);
    tracer->setGraph(pGraph);

    return tracer;
}

