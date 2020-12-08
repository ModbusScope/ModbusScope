
#include "graphviewmarkers.h"
#include "graphview.h"

GraphViewMarkers::GraphViewMarkers(GuiModel* pGuiModel, MyQCustomPlot* pPlot, QObject *parent) :
    QObject(parent),
    _pGuiModel(pGuiModel),
    _pPlot(pPlot),
    _pGraphview(dynamic_cast<GraphView*>(parent))
{
    QPen markerPen;
    markerPen.setWidth(2);

    markerPen.setColor(QColor(Qt::green));
    _pStartMarker = new QCPItemStraightLine(_pPlot);
    _pStartMarker->setVisible(false);
    _pStartMarker->setPen(markerPen);

    markerPen.setColor(QColor(Qt::red));
    _pEndMarker = new QCPItemStraightLine(_pPlot);
    _pEndMarker->setVisible(false);
    _pEndMarker->setPen(markerPen);

    connect(_pGuiModel, SIGNAL(markerStateChanged()), this, SLOT(updateMarkersVisibility()));
    connect(_pGuiModel, SIGNAL(startMarkerPosChanged()), this, SLOT(setStartMarker()));
    connect(_pGuiModel, SIGNAL(endMarkerPosChanged()), this, SLOT(setEndMarker()));
}

GraphViewMarkers::~GraphViewMarkers()
{

}

void GraphViewMarkers::updateMarkersVisibility()
{
    if (_pGuiModel->markerState() == false)
    {
        _pStartMarker->setVisible(false);
        _pEndMarker->setVisible(false);

        _pPlot->replot();
    }
}

void GraphViewMarkers::setStartMarker()
{
    _pStartMarker->setVisible(true);
    _pStartMarker->point1->setCoords(_pGuiModel->startMarkerPos(), 0);
    _pStartMarker->point2->setCoords(_pGuiModel->startMarkerPos(), 1);

    _pPlot->replot();
}

void GraphViewMarkers::setEndMarker()
{
    _pEndMarker->setVisible(true);
    _pEndMarker->point1->setCoords(_pGuiModel->endMarkerPos(), 0);
    _pEndMarker->point2->setCoords(_pGuiModel->endMarkerPos(), 1);

    _pPlot->replot();
}
