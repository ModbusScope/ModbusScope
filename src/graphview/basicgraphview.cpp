#include <QMessageBox>
#include <QVector>
#include <QtGlobal>
#include <QLocale>

#include <algorithm> // std::upperbound, std::lowerbound

#include "guimodel.h"
#include "util.h"
#include "graphdatamodel.h"
#include "myqcpaxistickertime.h"
#include "myqcpaxis.h"
#include "basicgraphview.h"

BasicGraphView::BasicGraphView(GuiModel * pGuiModel, GraphDataModel * pGraphDataModel, MyQCustomPlot * pPlot, QObject *parent) :
    QObject(parent)
{
    _pGuiModel = pGuiModel;
    _pGraphDataModel = pGraphDataModel;

   _pPlot = pPlot;

   /* Range drag is also enabled/disabled on mousePress and mouseRelease event */
   _pPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);

   // disable anti aliasing while dragging
   _pPlot->setNoAntialiasingOnDrag(true);

   /*
    * Greatly improves performance
    *
    * phFastPolylines	Graph/Curve lines are drawn with a faster method. This reduces the quality especially
    *                   of the line segment joins. (Only relevant for solid line pens.)
    * phCacheLabels		axis (tick) labels will be cached as pixmaps, increasing replot performance.
    * */
   _pPlot->setPlottingHints(QCP::phCacheLabels | QCP::phFastPolylines);

    // Replace y-axis with custom axis
   _pPlot->axisRect()->removeAxis(_pPlot->axisRect()->axes(QCPAxis::atLeft)[0]);
   _pPlot->axisRect()->addAxis(QCPAxis::atLeft, new MyQCPAxis(_pPlot->axisRect(), QCPAxis::atLeft));

    // Fix axis settings
    QCPAxis * pXAxis = _pPlot->axisRect()->axes(QCPAxis::atBottom)[0];
    QCPAxis * pYAxis = _pPlot->axisRect()->axes(QCPAxis::atLeft)[0];
    pYAxis->grid()->setVisible(true);
    _pPlot->axisRect()->setRangeDragAxes(pXAxis, pYAxis);
    _pPlot->axisRect()->setRangeZoomAxes(pXAxis, pYAxis);

    // Add custom axis ticker
   QSharedPointer<QCPAxisTickerTime> timeTicker(new MyQCPAxisTickerTime(_pPlot));
   _pPlot->xAxis->setTicker(timeTicker);
   _pPlot->xAxis->setLabel("Time");
   _pPlot->xAxis->setRange(0, 10000);

   _pPlot->yAxis->setRange(0, 65535);

   connect(_pPlot->xAxis, SIGNAL(rangeChanged(QCPRange, QCPRange)), this, SLOT(updateTooltip()));

   // Samples are enabled
   _bEnableSampleHighlight = true;

   // Add layer to move graph on front
   _pPlot->addLayer("topMain", _pPlot->layer("main"), QCustomPlot::limAbove);

   // connect slot that ties some axis selections together (especially opposite axes):
   connect(_pPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

   // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
   connect(_pPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress(QMouseEvent*)));
   connect(_pPlot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseRelease()));
   connect(_pPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
   connect(_pPlot, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(axisDoubleClicked(QCPAxis*)));
   connect(_pPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMove(QMouseEvent*)));
   connect(_pPlot, SIGNAL(beforeReplot()), this, SLOT(handleSamplePoints()));

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

   _pPlot->replot();

}

BasicGraphView::~BasicGraphView()
{

}

qint32 BasicGraphView::graphDataSize()
{
    return _pPlot->graph(0)->data()->size();
}

bool BasicGraphView::valuesUnderCursor(QList<double> &valueList)
{
    bool bRet = true;

    const double xPos = _pPlot->xAxis->pixelToCoord(_pPlot->mapFromGlobal(QCursor::pos()).x());

    if (_pPlot->graphCount() > 0)
    {
        QCPGraphDataContainer::const_iterator tooltipIt = getClosestPoint(xPos);

        bool bValid;
        const QCPRange keyRange = _pPlot->graph(0)->data()->keyRange(bValid);

        // Check all graphs
        for (qint32 activeGraphIndex = 0; activeGraphIndex < _pPlot->graphCount(); activeGraphIndex++)
        {
            if (
                    _pPlot->underMouse()
                    && bValid
                    && keyRange.contains(xPos)
                )
            {
                const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(activeGraphIndex);
                QCPGraphDataContainer::const_iterator graphDataIt = _pGraphDataModel->dataMap(graphIdx).data()->findBegin(tooltipIt->key, false);
                valueList.append(graphDataIt->value);
            }
            else
            {
                valueList.append(0);
                bRet = false;
            }
        }
    }
    else
    {
        valueList.append(0);
        bRet = false;
    }

    return bRet;

}

void BasicGraphView::manualScaleXAxis(qint64 min, qint64 max)
{
    _pPlot->xAxis->setRange(min, max);
    _pPlot->replot();
}

void BasicGraphView::manualScaleYAxis(qint64 min, qint64 max)
{
    _pPlot->yAxis->setRange(min, max);
    _pPlot->replot();
}

void BasicGraphView::autoScaleXAxis()
{
    _pPlot->xAxis->rescale(true);
    _pPlot->replot();
}

void BasicGraphView::autoScaleYAxis()
{
    _pPlot->yAxis->rescale(true);
    _pPlot->replot();
}

void BasicGraphView::updateTooltip()
{
    paintTimeStampToolTip(_pPlot->mapFromGlobal(QCursor::pos()));
}

void BasicGraphView::enableSamplePoints()
{
    _bEnableSampleHighlight = _pGuiModel->highlightSamples();
    _pPlot->replot();
}

void BasicGraphView::clearGraph(const quint32 graphIdx)
{
    if (_pGraphDataModel->isActive(graphIdx))
    {

        QList<quint16> activeGraphList;
        _pGraphDataModel->activeGraphIndexList(&activeGraphList);

        if (activeGraphList.size() <= 0)
        {
            // No active graph: not possible
        }
        else if (activeGraphList.size() == 1)
        {
            /* Only one graph active: clear all data */
            _pGraphDataModel->dataMap(graphIdx)->clear();

            _pPlot->replot();
        }
        else
        {
            /* Several active graph, keep time data but clear data */
            QCPGraphDataContainer::iterator it = _pGraphDataModel->dataMap(graphIdx)->begin();

            /* Clear all values, keep keys */
            while(it != _pGraphDataModel->dataMap(graphIdx)->end())
            {
                it->value = 0u;
                it++;
            }

            _pPlot->replot();
        }
    }
}

void BasicGraphView::updateGraphs()
{
    /* Clear graphs and add current active graphs */
    _pPlot->clearGraphs();

    QList<quint16> activeGraphList;
    _pGraphDataModel->activeGraphIndexList(&activeGraphList);

    if (activeGraphList.size() > 0)
    {
        // All graphs should have the same amount of points.
        // Loop over graphs and get maximum count of samples
        qint32 maxSampleCount = 0;
        quint32 maxSampleIdx = 0;

        foreach(quint16 graphIdx, activeGraphList)
        {
            const qint32 sampleCount = _pGraphDataModel->dataMap(graphIdx)->size();
            if (sampleCount > maxSampleCount)
            {
                maxSampleCount = sampleCount;
                maxSampleIdx = graphIdx;
            }
        }

        // Graph that have less points will be zeroed with that amount of points
        foreach(quint16 graphIdx, activeGraphList)
        {
            // Add graph
            QCPGraph * pGraph = _pPlot->addGraph();

            pGraph->setName(_pGraphDataModel->label(graphIdx));

            QPen pen;
            pen.setColor(_pGraphDataModel->color(graphIdx));
            pen.setWidth(2);
            pen.setCosmetic(true);

            pGraph->setPen(pen);


            QSharedPointer<QCPGraphDataContainer> pMap = _pGraphDataModel->dataMap(graphIdx);

            // Set data to zero when needed
            if (pMap->size() != maxSampleCount)
            {
                const QSharedPointer<QCPGraphDataContainer> pReferenceMap = _pGraphDataModel->dataMap(maxSampleIdx);
                pMap->clear();

                // Add zero value for every key (x-coordinate)
                QCPGraphDataContainer::const_iterator refIt = pReferenceMap->constBegin();
                while(refIt != pReferenceMap->constEnd())
                {
                    pMap->add(QCPGraphData(refIt->key, 0));
                    refIt++;
                }
            }

            // Set graph datamap
            pGraph->setData(pMap);
        }
    }

    _pPlot->replot();
}

void BasicGraphView::changeGraphColor(const quint32 graphIdx)
{
    if (_pGraphDataModel->isActive(graphIdx))
    {
        const quint32 activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

        QPen pen;
        pen.setColor(_pGraphDataModel->color(graphIdx));
        pen.setWidth(2);
        pen.setCosmetic(true);

        _pPlot->graph(activeIdx)->setPen(pen);

        _pPlot->replot();
    }
}

void BasicGraphView::changeGraphLabel(const quint32 graphIdx)
{
    if (_pGraphDataModel->isActive(graphIdx))
    {
        const quint32 activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

        _pPlot->graph(activeIdx)->setName(_pGraphDataModel->label(graphIdx));

        _pPlot->replot();
    }
}

void BasicGraphView::bringToFront()
{
    if (_pPlot->graphCount() > 0)
    {
        _pPlot->graph(_pGuiModel->frontGraph())->setLayer("topMain");
        _pPlot->replot();
    }
}

void BasicGraphView::updateMarkersVisibility()
{
    if (_pGuiModel->markerState() == false)
    {
        _pStartMarker->setVisible(false);
        _pEndMarker->setVisible(false);

        _pPlot->replot();
    }
}

void BasicGraphView::setStartMarker()
{
    _pStartMarker->setVisible(true);
    _pStartMarker->point1->setCoords(_pGuiModel->startMarkerPos(), 0);
    _pStartMarker->point2->setCoords(_pGuiModel->startMarkerPos(), 1);

    _pPlot->replot();
}

void BasicGraphView::setEndMarker()
{
    _pEndMarker->setVisible(true);
    _pEndMarker->point1->setCoords(_pGuiModel->endMarkerPos(), 0);
    _pEndMarker->point2->setCoords(_pGuiModel->endMarkerPos(), 1);

    _pPlot->replot();
}

void BasicGraphView::setOpenGl(bool bState)
{
    _pPlot->setOpenGl(bState);
}

bool BasicGraphView::openGl(void)
{
    return _pPlot->openGl();
}

void BasicGraphView::selectionChanged()
{
   /*
   normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
   the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
   and the axis base line together. However, the axis label shall be selectable individually.
   */

   // handle axis and tick labels as one selectable object:
   if (_pPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || _pPlot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) || _pPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxisLabel))
   {
       _pPlot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spAxisLabel|QCPAxis::spTickLabels);
   }
   // handle axis and tick labels as one selectable object:
   if (_pPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || _pPlot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) || _pPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxisLabel))
   {
       _pPlot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spAxisLabel|QCPAxis::spTickLabels);
   }

}

void BasicGraphView::mousePress(QMouseEvent *event)
{
   if (event->modifiers() & Qt::ControlModifier)
   {
       /* Disable range drag when control key is pressed */
       _pPlot->setInteraction(QCP::iRangeDrag, false);
       _pPlot->setInteraction(QCP::iRangeZoom, false);

       if (_pPlot->graphCount() > 0)
       {
           const double xPos = _pPlot->xAxis->pixelToCoord(event->pos().x());
           QCPGraphDataContainer::const_iterator markerPosIt = getClosestPoint(xPos);

           if (event->button() & Qt::LeftButton)
           {
                _pGuiModel->setStartMarkerPos(markerPosIt->key);
           }
           else if (event->button() & Qt::RightButton)
           {
                _pGuiModel->setEndMarkerPos(markerPosIt->key);
           }
           else
           {
               // No function
           }
       }
   }
   else
   {
       // if an axis is selected, only allow the direction of that axis to be dragged
       // if no axis is selected, both directions may be dragged

       if (_pPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
       {
           _pPlot->axisRect()->setRangeDrag(_pPlot->xAxis->orientation());
       }
       else if (_pPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
       {
           _pPlot->axisRect()->setRangeDrag(_pPlot->yAxis->orientation());
       }
       else
       {
           _pPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
       }
   }
}

void BasicGraphView::mouseRelease()
{
    /* Always re-enable range drag */
    _pPlot->setInteraction(QCP::iRangeDrag, true);
    _pPlot->setInteraction(QCP::iRangeZoom, true);
}

void BasicGraphView::mouseWheel()
{
   // if an axis is selected, only allow the direction of that axis to be zoomed
   // if no axis is selected, both directions may be zoomed

   if (_pPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
   {
       _pPlot->axisRect()->setRangeZoom(_pPlot->xAxis->orientation());
       _pGuiModel->setxAxisScale(SCALE_MANUAL); // change to manual scaling
   }
   else if (_pPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
   {
       _pPlot->axisRect()->setRangeZoom(_pPlot->yAxis->orientation());
       _pGuiModel->setyAxisScale(SCALE_MANUAL); // change to manual scaling
   }
   else
   {
       _pPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
       _pGuiModel->setyAxisScale(SCALE_MANUAL); // change to manual scaling
       _pGuiModel->setxAxisScale(SCALE_MANUAL);
   }
}

void BasicGraphView::mouseMove(QMouseEvent *event)
{
    // Check for graph drag
    if(event->buttons() & Qt::LeftButton)
    {
        if (!(event->modifiers() & Qt::ControlModifier))
        {
            if (_pPlot->axisRect()->rangeDrag() == Qt::Horizontal)
            {
                _pGuiModel->setxAxisScale(SCALE_MANUAL); // change to manual scaling
            }
            else if (_pPlot->axisRect()->rangeDrag() == Qt::Vertical)
            {
                _pGuiModel->setyAxisScale(SCALE_MANUAL); // change to manual scaling
            }
            else
            {
                // Both change to manual scaling
                _pGuiModel->setxAxisScale(SCALE_MANUAL);
                _pGuiModel->setyAxisScale(SCALE_MANUAL);
            }
        }
    }
    else
    {
        paintTimeStampToolTip(event->pos());

        if (_pGuiModel->cursorValues())
        {
            emit cursorValueUpdate();
        }
    }
}

void BasicGraphView::paintTimeStampToolTip(QPoint pos)
{

    if  (_pGuiModel->cursorValues() && (_pPlot->graphCount() > 0))
    {

        const double xPos = _pPlot->xAxis->pixelToCoord(pos.x());
        QCPGraphDataContainer::const_iterator tooltipIt = getClosestPoint(xPos);

        bool bValid;
        const QCPRange keyRange = _pPlot->graph(0)->data()->keyRange(bValid);

        if (bValid && keyRange.contains(xPos))
        {
            // Add tick key string
            QString toolText = Util::formatTime(tooltipIt->key, false);

            QToolTip::showText(_pPlot->mapToGlobal(pos), toolText, _pPlot);

        }
        else
        {
            // Hide tooltip
            QToolTip::hideText();
        }
    }
    else
    {
        if (QToolTip::isVisible())
        {
            QToolTip::hideText();
        }
    }
}

void BasicGraphView::handleSamplePoints()
{
    bool bHighlight = false;

    if (_bEnableSampleHighlight)
    {
        if (_pPlot->graphCount() > 0 && (graphDataSize() > 0))
        {
            QCPRange axisRange = _pPlot->xAxis->range();

            auto lowerBoundIt = _pPlot->graph(0)->data()->findBegin(axisRange.lower, false);
            auto upperBoundIt = _pPlot->graph(0)->data()->findBegin(axisRange.upper);

            const int pointCount = upperBoundIt - lowerBoundIt;

            /* Get size in pixels */
            const double sizePx = _pPlot->xAxis->coordToPixel(upperBoundIt->key) - _pPlot->xAxis->coordToPixel(lowerBoundIt->key);

            /* Calculate number of pixels per point */
            double nrOfPixelsPerPoint;

            if (lowerBoundIt != upperBoundIt)
            {
                nrOfPixelsPerPoint = sizePx / qAbs(pointCount);
            }
            else
            {
                nrOfPixelsPerPoint = sizePx;
            }

            if (nrOfPixelsPerPoint > _cPixelPerPointThreshold)
            {
                bHighlight = true;
            }

        }
    }

    /* TODO: add hysteresis to highlight sample points */
    highlightSamples(bHighlight);
}

void BasicGraphView::axisDoubleClicked(QCPAxis * axis)
{
    if (axis == _pPlot->xAxis)
    {
        _pGuiModel->setxAxisScale(SCALE_AUTO);
    }
    else if (axis == _pPlot->yAxis)
    {
        _pGuiModel->setyAxisScale(SCALE_AUTO);
    }
    else
    {
        // do nothing
    }
}

void BasicGraphView::highlightSamples(bool bState)
{
    for (qint32 graphIndex = 0; graphIndex < _pPlot->graphCount(); graphIndex++)
    {
        if (bState)
        {
            _pPlot->graph(graphIndex)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));
        }
        else
        {
            _pPlot->graph(graphIndex)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone));
        }
    }
}

qint32 BasicGraphView::graphIndex(QCPGraph * pGraph)
{
    qint32 ret = -1;

    for (qint32 graphIndex = 0; graphIndex < _pPlot->graphCount(); graphIndex++)
    {
        if (pGraph == _pPlot->graph(graphIndex))
        {
            ret = graphIndex;
            break;
        }
    }

    return ret;
}

QCPGraphDataContainer::const_iterator BasicGraphView::getClosestPoint(double xPos)
{
    QCPGraphDataContainer::const_iterator closestIt = _pPlot->graph(0)->data()->constBegin();
    QCPGraphDataContainer::const_iterator leftIt = _pPlot->graph(0)->data()->findBegin(xPos);

    auto rightIt = leftIt + 1;
    if (rightIt !=  _pPlot->graph(0)->data()->constEnd())
    {

        const double diffReference = rightIt->key - leftIt->key;
        const double diffPos = xPos - leftIt->key;

        if (diffPos > (diffReference / 2))
        {
            closestIt = rightIt;
        }
        else
        {
            closestIt = leftIt;
        }
    }
    else
    {
        closestIt = leftIt;
    }

    return closestIt;
}
