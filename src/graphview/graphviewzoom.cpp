
#include "graphviewzoom.h"

GraphViewZoom::GraphViewZoom(GuiModel* pGuiModel, MyQCustomPlot* pPlot, QObject *parent) :
    QObject(parent)
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
    if (_pGuiModel->zoomState() != GuiModel::ZOOM_IDLE)
    {
        _pPlot->setCursor(Qt::CrossCursor);
    }
    else
    {
        _pPlot->setCursor(Qt::ArrowCursor);
    }
}

/*!
 * \brief GraphViewZoom::handleMousePress
 * \param event
 * \return true when event already handled
 */
bool GraphViewZoom::handleMousePress(QMouseEvent *event)
{
    if (_pGuiModel->zoomState() == GuiModel::ZOOM_TRIGGERED)
    {
        /* Disable range drag when zooming is active */
        _pPlot->setInteraction(QCP::iRangeDrag, false);
        _pPlot->setInteraction(QCP::iRangeZoom, false);

        _pGuiModel->setZoomState(GuiModel::ZOOM_SELECTING);

        /* Save coordinates */
        _startX = event->pos().x();
        _startY = event->pos().y();

        return true;
    }

    return false;
}

bool GraphViewZoom::handleMouseWheel()
{
    if (_pGuiModel->zoomState() != GuiModel::ZOOM_IDLE)
    {
        /* Zoom has precedence when not idle */
        return true;
    }

    return false;

}

bool GraphViewZoom::handleMouseRelease(QMouseEvent *event)
{
    if (_pGuiModel->zoomState() == GuiModel::ZOOM_SELECTING)
    {
        /* Do zoom */

        _pGuiModel->setZoomState(GuiModel::ZOOM_IDLE);

        return true;
    }

    return false;
}


bool GraphViewZoom::handleMouseMove(QMouseEvent *event)
{
    if (_pGuiModel->zoomState() == GuiModel::ZOOM_SELECTING)
    {
        /* Draw rectangle */


        return true;
    }

    return false;
}
