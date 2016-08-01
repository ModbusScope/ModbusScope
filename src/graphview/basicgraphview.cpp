#include <QMessageBox>
#include <QVector>
#include <QtGlobal>
#include <QLocale>

#include <algorithm> // std::upperbound, std::lowerbound

#include "guimodel.h"
#include "util.h"
#include "graphdatamodel.h"
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
    * phForceRepaint	causes an immediate repaint() instead of a soft update() when QCustomPlot::replot()
    *                   is called with parameter QCustomPlot::rpHint. This is set by default to prevent the
    *                   plot from freezing on fast consecutive replots (e.g. user drags ranges with mouse).
    * phCacheLabels		axis (tick) labels will be cached as pixmaps, increasing replot performance.
    * */
   _pPlot->setPlottingHints(QCP::phCacheLabels | QCP::phFastPolylines | QCP::phForceRepaint);

   _pPlot->xAxis->setTickLabelType(QCPAxis::ltNumber);
   _pPlot->xAxis->setNumberFormat("gb");
   _pPlot->xAxis->setRange(0, 10000);
   _pPlot->xAxis->setAutoTicks(true);
   _pPlot->xAxis->setAutoTickLabels(false);
   _pPlot->xAxis->setLabel("Time (s)");
   connect(_pPlot->xAxis, SIGNAL(ticksRequest()), this, SLOT(generateTickLabels()));

   _pPlot->yAxis->setRange(0, 65535);

   // Tooltip is enabled
   _bEnableTooltip = true;

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
   _pPlot->addItem(_pStartMarker);
   _pStartMarker->setVisible(false);
   _pStartMarker->setPen(markerPen);

   markerPen.setColor(QColor(Qt::red));
   _pEndMarker = new QCPItemStraightLine(_pPlot);
   _pPlot->addItem(_pEndMarker);
   _pEndMarker->setVisible(false);
   _pEndMarker->setPen(markerPen);

   _pPlot->replot();

}

BasicGraphView::~BasicGraphView()
{

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

void BasicGraphView::enableValueTooltip()
{
    _bEnableTooltip = _pGuiModel->valueTooltip();
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
            QCPDataMap * pMap = _pGraphDataModel->dataMap(graphIdx);
            pMap->clear();

            _pPlot->replot();
        }
        else
        {
            /* Several active graph, keep time data but clear data */

            QCPDataMap * pMap = _pGraphDataModel->dataMap(graphIdx);

            /* Clear all values, keep keys */
            QMutableMapIterator<double, QCPData> i(*pMap);
            while (i.hasNext())
            {
                i.next();

                i.setValue(QCPData(i.key(), 0));
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
            const qint32 sampleCount = _pGraphDataModel->dataMap(graphIdx)->keys().size();
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
            MyQCPGraph * pGraph = _pPlot->addCustomGraph();

            pGraph->setName(_pGraphDataModel->label(graphIdx));

            QPen pen;
            pen.setColor(_pGraphDataModel->color(graphIdx));
            pen.setWidth(2);
            pen.setCosmetic(true);

            pGraph->setPen(pen);


            QCPDataMap * pMap = _pGraphDataModel->dataMap(graphIdx);

            // Set data to zero when needed
            if (_pGraphDataModel->dataMap(graphIdx)->keys().size() != maxSampleCount)
            {
                const QCPDataMap * pReferenceMap = _pGraphDataModel->dataMap(maxSampleIdx);
                pMap->clear();

                // Add zero value for every key (x-coordinate)
                foreach(double key, pReferenceMap->keys())
                {
                    pMap->insert(key, QCPData(key, 0));
                }
            }

            // Set graph datamap
            pGraph->setData(pMap, false);
        }
    }

    _pPlot->replot();
}

void BasicGraphView::showGraph(quint32 graphIdx)
{    
    if (_pGraphDataModel->isActive(graphIdx))
    {
        const bool bShow = _pGraphDataModel->isVisible(graphIdx);

        const quint32 activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

        _pPlot->graph(activeIdx)->setVisible(bShow);

        _pPlot->replot();
    }
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

void BasicGraphView::generateTickLabels()
{
    QVector<double> ticks = _pPlot->xAxis->tickVector();

    /* Clear ticks vector */
    tickLabels.clear();

    const bool bSmallScale = smallScaleActive(ticks);

    /* Generate correct labels */
    for (qint32 index = 0; index < ticks.size(); index++)
    {
        tickLabels.append(Util::formatTime(ticks[index], bSmallScale));
    }

    /* Set labels */
    _pPlot->xAxis->setTickVectorLabels(tickLabels);
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

       const double xPos = _pPlot->xAxis->pixelToCoord(event->pos().x());

       double correctXPos = 0;
       if (_pPlot->graphCount() > 0)
       {
           const QList<double> keyList = _pPlot->graph(0)->data()->keys();

           // find the nearest point
           for (qint32 i = 1; i < keyList.size(); i++)
           {
               const double leftPoint = keyList[i - 1];
               const double rightPoint = keyList[i];

               if (
                   (xPos > leftPoint)
                   && (xPos <= rightPoint)
                   )
               {
                   const double xCoordPxl = xPos - leftPoint;
                   const double diff = rightPoint - leftPoint;

                   if (xCoordPxl > diff / 2)
                   {
                       correctXPos = rightPoint;
                   }
                   else
                   {
                       correctXPos = leftPoint;
                   }

                   break;
               }
           }
       }

       if (event->button() & Qt::LeftButton)
       {
            _pGuiModel->setStartMarkerPos(correctXPos);
       }
       else if (event->button() & Qt::RightButton)
       {
            _pGuiModel->setEndMarkerPos(correctXPos);
       }
       else
       {
           // No function
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
        paintValueToolTip(event);
    }
}

void BasicGraphView::paintValueToolTip(QMouseEvent *event)
{
    if  (_bEnableTooltip)
    {
        const double xPos = _pPlot->xAxis->pixelToCoord(event->pos().x());

        if (_pPlot->graphCount() > 0)
        {
            QString toolText;
            const QList<double> keyList = _pPlot->graph(0)->data()->keys();

            // Find if cursor is in range to show tooltip
            qint32 i = 0;
            bool bInRange = false;
            for (i = 1; i < keyList.size(); i++)
            {
                // find the two nearest points
                if (
                    (xPos > keyList[i - 1])
                    && (xPos <= keyList[i])
                    )
                {
                    const double leftPointPxl = _pPlot->xAxis->coordToPixel(keyList[i - 1]);
                    const double rightPointPxl = _pPlot->xAxis->coordToPixel(keyList[i]);
                    const double xCoordPxl = event->pos().x();
                    double keyIndex = -1;

                    if (
                        (xCoordPxl >= leftPointPxl)
                        && (xCoordPxl <= (leftPointPxl + _cPixelNearThreshold))
                        )
                    {
                        keyIndex = i - 1;
                    }
                    else if (
                         (xCoordPxl >= (rightPointPxl - _cPixelNearThreshold))
                         && (xCoordPxl <= rightPointPxl)
                        )
                    {
                        keyIndex = i;
                    }
                    else
                    {
                        // no point near enough
                        keyIndex = -1;
                    }

                    if (keyIndex != -1)
                    {
                        bInRange = true;

                        const bool bSmallScale = smallScaleActive(keyList.toVector());

                        // Add tick key string
                        toolText = Util::formatTime(keyList[keyIndex], bSmallScale);

                        // Check all graphs
                        for (qint32 activeGraphIndex = 0; activeGraphIndex < _pPlot->graphCount(); activeGraphIndex++)
                        {
                            if (_pPlot->graph(activeGraphIndex)->visible())
                            {
                                const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(activeGraphIndex);
                                const double value = _pGraphDataModel->dataMap(graphIdx)->values()[keyIndex].value;

                                toolText += QString("\n%1: %2").arg(_pGraphDataModel->label(graphIdx)).arg(value);
                            }
                        }
                        break;
                    }
                }
            }

            if (!bInRange)
            {
                // Hide tooltip
                QToolTip::hideText();
            }
            else
            {
                QToolTip::showText(_pPlot->mapToGlobal(event->pos()), toolText, _pPlot);
            }
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
        if (_pPlot->graphCount() > 0 && _pPlot->graph(0)->data()->size() > 0)
        {
            /* Get key from visible lower bound */
            double lowerBoundKey;
            if (_pPlot->xAxis->range().lower <= _pPlot->graph(0)->data()->keys().first())
            {
                lowerBoundKey = _pPlot->graph(0)->data()->keys().first();
            }
            else
            {
                lowerBoundKey = _pPlot->graph(0)->data()->lowerBound(_pPlot->xAxis->range().lower).key();
            }

            /* Get key from visible upper bound */
            double upperBoundKey;
            if (_pPlot->xAxis->range().upper >= _pPlot->graph(0)->data()->keys().last())
            {
                upperBoundKey = _pPlot->graph(0)->data()->keys().last();
            }
            else
            {
                upperBoundKey = _pPlot->graph(0)->data()->upperBound(_pPlot->xAxis->range().upper).key();
            }

            /* get indexes of keys */
            const quint32 lowerBoundIndex = _pPlot->graph(0)->data()->keys().indexOf(lowerBoundKey);
            const quint32 upperBoundIndex = _pPlot->graph(0)->data()->keys().lastIndexOf(upperBoundKey);

            /* Get size in pixels */
            const double sizePx = _pPlot->xAxis->coordToPixel(upperBoundKey) - _pPlot->xAxis->coordToPixel(lowerBoundKey);

            /* Calculate number of pixels per point */
            const double nrOfPixelsPerPoint = sizePx / qAbs(upperBoundIndex - lowerBoundIndex);

            if (nrOfPixelsPerPoint > _cPixelPerPointThreshold)
            {
                bHighlight = true;
            }
        }
    }

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

bool BasicGraphView::smallScaleActive(QVector<double> tickList)
{
    bool bRet = false;
    if (qAbs(tickList.last() - tickList.first()) < _cSmallScaleDiff)
    {
        bRet = true;
    }

    if (bRet)
    {
        _pPlot->xAxis->setLabel("Time (ms)");
    }
    else
    {
        _pPlot->xAxis->setLabel("Time (s)");
    }

    return bRet;
}
