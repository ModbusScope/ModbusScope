
#include "graphviewzoom.h"

GraphViewZoom::GraphViewZoom(GuiModel* pGuiModel, MyQCustomPlot* pPlot)
{
    _pGuiModel = pGuiModel;
    _pPlot = pPlot;

    connect(_pGuiModel, SIGNAL(zoomStateChanged()), this, SLOT(handleZoomStateChanged()));
}

GraphViewZoom::~GraphViewZoom()
{

}

void GraphViewZoom::handleZoomStateChanged()
{
    if (_pGuiModel->zoomState() == GuiModel::ZOOM_SELECTING)
    {
        _pPlot->setCursor(Qt::CrossCursor);
    }
    else
    {
        _pPlot->setCursor(Qt::ArrowCursor);
    }
}

