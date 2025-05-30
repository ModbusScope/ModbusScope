
#include "graphviewzoom.h"

#include "graphview/graphview.h"
#include "graphview/scopeplot.h"
#include "models/guimodel.h"

GraphViewZoom::GraphViewZoom(GuiModel* pGuiModel, ScopePlot* pPlot, QObject *parent) :
    QObject(parent)
{
    _pRubberBand = nullptr;
    _pGuiModel = pGuiModel;
    _pGraphview = dynamic_cast<GraphView*>(parent);
    _pPlot = pPlot;

    connect(_pGuiModel, &GuiModel::zoomStateChanged, this, &GraphViewZoom::handleZoomStateChanged);
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
        _pGuiModel->setZoomState(GuiModel::ZOOM_SELECTING);

        _selectionOrigin = event->pos();
        if (!_pRubberBand)
        {
            _pRubberBand = new QRubberBand(QRubberBand::Rectangle, _pPlot);
        }
        _pRubberBand->setGeometry(QRect(_selectionOrigin, QSize()));
        _pRubberBand->show();

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

bool GraphViewZoom::handleMouseRelease()
{
    if (_pGuiModel->zoomState() == GuiModel::ZOOM_SELECTING)
    {
        _pRubberBand->hide();

        _pGuiModel->setyAxisScale(AxisMode::SCALE_MANUAL); // change to manual scaling
        _pGuiModel->sety2AxisScale(AxisMode::SCALE_MANUAL); // change to manual scaling
        _pGuiModel->setxAxisScale(AxisMode::SCALE_MANUAL);

        /* Perform zoom based on selected rubberband */
        performZoom();

        /* Reset state */
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
        _pRubberBand->setGeometry(QRect(_selectionOrigin, event->pos()).normalized());

        return true;
    }

    return false;
}

void GraphViewZoom::performZoom(void)
{
    const double correctTopLeftX = _pGraphview->pixelToClosestKey(_pRubberBand->geometry().topLeft().x());
    const double correctTopLeftY = _pPlot->yAxis->pixelToCoord(_pRubberBand->geometry().topLeft().y());
    const double correctTopLeftY2 = _pPlot->yAxis2->pixelToCoord(_pRubberBand->geometry().topLeft().y());

    const double correctBottomRightX = _pGraphview->pixelToClosestKey(_pRubberBand->geometry().bottomRight().x());
    const double correctBottomRightY = _pPlot->yAxis->pixelToCoord(_pRubberBand->geometry().bottomRight().y());
    const double correctBottomRightY2 = _pPlot->yAxis2->pixelToCoord(_pRubberBand->geometry().bottomRight().y());

    _pPlot->xAxis->setRange(correctTopLeftX, correctBottomRightX);
    _pPlot->yAxis->setRange(correctBottomRightY, correctTopLeftY);
    _pPlot->yAxis2->setRange(correctBottomRightY2, correctTopLeftY2);
}

