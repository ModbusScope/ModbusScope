
#include <QVector>
#include <QtGlobal>
#include <QLocale>
#include <QInputDialog>

#include <algorithm> // std::upperbound, std::lowerbound

#include "guimodel.h"
#include "util.h"
#include "graphdatamodel.h"
#include "settingsmodel.h"
#include "notemodel.h"
#include "myqcpaxistickertime.h"
#include "myqcpaxis.h"
#include "graphview.h"
#include "graphviewzoom.h"
#include "notehandling.h"

GraphView::GraphView(GuiModel * pGuiModel, SettingsModel *pSettingsModel, GraphDataModel * pGraphDataModel, NoteModel *pNoteModel, MyQCustomPlot * pPlot, QObject *parent) :
    QObject(parent)
{
    _pGuiModel = pGuiModel;
    _pGraphDataModel = pGraphDataModel;
    _pSettingsModel = pSettingsModel;

   _pPlot = pPlot;

   _pGraphViewZoom = new GraphViewZoom(_pGuiModel, _pPlot, this);
   _pNoteHandling = new NoteHandling(pNoteModel, _pPlot, this);

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
   connect(_pPlot->xAxis, SIGNAL(rangeChanged(QCPRange, QCPRange)), this, SLOT(xAxisRangeChanged(QCPRange, QCPRange)));

   // Samples are enabled
   _bEnableSampleHighlight = true;

   // Add layer to move graph on front
   _pPlot->addLayer("topMain", _pPlot->layer("main"), QCustomPlot::limAbove);

   // connect slot that ties some axis selections together (especially opposite axes):
   connect(_pPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

   // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
   connect(_pPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress(QMouseEvent*)));
   connect(_pPlot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseRelease(QMouseEvent*)));
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

GraphView::~GraphView()
{
    delete _pGraphViewZoom;
}

qint32 GraphView::graphDataSize()
{
    return _pPlot->graph(0)->data()->size();
}

bool GraphView::valuesUnderCursor(QList<double> &valueList)
{
    bool bRet = true;

    const double xPos = _pPlot->xAxis->pixelToCoord(_pPlot->mapFromGlobal(QCursor::pos()).x());

    if (_pPlot->graphCount() > 0)
    {
        double tooltipPos = getClosestPoint(xPos);

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
                QCPGraphDataContainer::const_iterator graphDataIt = _pGraphDataModel->dataMap(graphIdx).data()->findBegin(tooltipPos, false);
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

QPointF GraphView::pixelToPointF(const QPoint& pixel) const
{
    return QPointF(_pPlot->xAxis->pixelToCoord(pixel.x()),
                   _pPlot->yAxis->pixelToCoord(pixel.y()));
}

double GraphView::pixelToClosestKey(double pixel)
{
    return getClosestPoint(_pPlot->xAxis->pixelToCoord(pixel));
}

double GraphView::pixelToClosestValue(double pixel)
{
    return getClosestPoint(_pPlot->yAxis->pixelToCoord(pixel));
}

void GraphView::updateTooltip()
{
    paintTimeStampToolTip(_pPlot->mapFromGlobal(QCursor::pos()));
}

void GraphView::enableSamplePoints()
{
    _bEnableSampleHighlight = _pGuiModel->highlightSamples();
    _pPlot->replot();
}

void GraphView::clearGraph(const quint32 graphIdx)
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

void GraphView::updateGraphs()
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

            /* Keep visibility state */
            /* TODO: partial duplicate of showGraph in ExtendedGraph */
            const bool bShow = _pGraphDataModel->isVisible(graphIdx);
            pGraph->setVisible(bShow);

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

void GraphView::changeGraphColor(const quint32 graphIdx)
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

void GraphView::changeGraphLabel(const quint32 graphIdx)
{
    if (_pGraphDataModel->isActive(graphIdx))
    {
        const quint32 activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

        _pPlot->graph(activeIdx)->setName(_pGraphDataModel->label(graphIdx));

        _pPlot->replot();
    }
}

void GraphView::bringToFront()
{
    if (_pPlot->graphCount() > 0)
    {
        _pPlot->graph(_pGuiModel->frontGraph())->setLayer("topMain");
        _pPlot->replot();
    }
}

void GraphView::updateMarkersVisibility()
{
    if (_pGuiModel->markerState() == false)
    {
        _pStartMarker->setVisible(false);
        _pEndMarker->setVisible(false);

        _pPlot->replot();
    }
}

void GraphView::setStartMarker()
{
    _pStartMarker->setVisible(true);
    _pStartMarker->point1->setCoords(_pGuiModel->startMarkerPos(), 0);
    _pStartMarker->point2->setCoords(_pGuiModel->startMarkerPos(), 1);

    _pPlot->replot();
}

void GraphView::setEndMarker()
{
    _pEndMarker->setVisible(true);
    _pEndMarker->point1->setCoords(_pGuiModel->endMarkerPos(), 0);
    _pEndMarker->point2->setCoords(_pGuiModel->endMarkerPos(), 1);

    _pPlot->replot();
}

void GraphView::addData(QList<double> timeData, QList<QList<double> > data)
{
    _pGuiModel->setxAxisScale(AxisMode::SCALE_AUTO);
    _pGuiModel->setyAxisScale(AxisMode::SCALE_AUTO);

    updateData(&timeData, &data);
}

void GraphView::showGraph(quint32 graphIdx)
{
    if (_pGraphDataModel->isActive(graphIdx))
    {
        const bool bShow = _pGraphDataModel->isVisible(graphIdx);

        const quint32 activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

        _pPlot->graph(activeIdx)->setVisible(bShow);

        rescalePlot();
    }
}

void GraphView::rescalePlot()
{

    // scale x-axis
    if (_pGuiModel->xAxisScalingMode() == AxisMode::SCALE_AUTO)
    {
        if ((_pPlot->graphCount() != 0) && (graphDataSize() != 0))
        {
            _pPlot->xAxis->rescale(true);
        }
        else
        {
            _pPlot->xAxis->setRange(0, 10000);
        }
    }
    else if (_pGuiModel->xAxisScalingMode() == AxisMode::SCALE_SLIDING)
    {
        // sliding window scale routine
        const quint64 slidingInterval = static_cast<quint64>(_pGuiModel->xAxisSlidingSec()) * 1000;
        if ((_pPlot->graphCount() != 0) && (graphDataSize() != 0))
        {
            auto lastDataIt = _pPlot->graph(0)->data()->constEnd();
            lastDataIt--; /* Point to last existing item */

            const quint64 lastTime = (quint64)lastDataIt->key;
            if (lastTime > slidingInterval)
            {
                _pPlot->xAxis->setRange(lastTime - slidingInterval, lastTime);
            }
            else
            {
                _pPlot->xAxis->setRange(0, slidingInterval);
            }
        }
        else
        {
            _pPlot->xAxis->setRange(0, slidingInterval);
        }
    }
    else // Manual
    {

    }

    // scale y-axis
    if (_pGuiModel->yAxisScalingMode() == AxisMode::SCALE_AUTO)
    {
        if ((_pPlot->graphCount() != 0) && (graphDataSize()))
        {
            _pPlot->yAxis->rescale(true);
        }
        else
        {
            _pPlot->yAxis->setRange(0, 10);
        }
    }
    else if (_pGuiModel->yAxisScalingMode() == AxisMode::SCALE_MINMAX)
    {
        // min max scale routine
        _pPlot->yAxis->setRange(_pGuiModel->yAxisMin(), _pGuiModel->yAxisMax());
    }
    else if (_pGuiModel->yAxisScalingMode() == AxisMode::SCALE_WINDOW_AUTO)
    {
        auto pAxis = dynamic_cast<MyQCPAxis *>(_pPlot->yAxis);
        if (pAxis != nullptr)
        {
            pAxis->rescaleValue(_pPlot->xAxis->range());
        }
    }
    else // Manual
    {

    }

    _pPlot->replot();
}

void GraphView::plotResults(QList<bool> successList, QList<double> valueList)
{
    /* QList correspond with activeGraphList */

    quint64 timeData;
    if (_pSettingsModel->absoluteTimes())
    {
        // Epoch is in UTC time
        timeData = QDateTime::currentMSecsSinceEpoch();
    }
    else
    {
        timeData = QDateTime::currentMSecsSinceEpoch() - _pGuiModel->communicationStartTime();
    }

    QList<double> dataList;

    for (qint32 i = 0; i < valueList.size(); i++)
    {
        if (successList[i])
        {
            // No error, add points
            _pPlot->graph(i)->addData(timeData, valueList[i]);
            dataList.append(valueList[i]);
        }
        else
        {
            _pPlot->graph(i)->addData(timeData, 0);
            dataList.append(0);
        }
    }

    emit dataAddedToPlot(timeData, dataList);

   rescalePlot();
}

void GraphView::clearResults()
{
    for (qint32 i = 0; i < _pPlot->graphCount(); i++)
    {
        _pPlot->graph(i)->data()->clear();
        _pPlot->graph(i)->setName(QString("(-) %1").arg(_pGraphDataModel->label(i)));
    }

   rescalePlot();
}

void GraphView::selectionChanged()
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

void GraphView::showMarkers()
{
    double startPos = (_pPlot->xAxis->range().size() * 1 / 3) + _pPlot->xAxis->range().lower;
    double endPos = (_pPlot->xAxis->range().size() * 2 / 3) + _pPlot->xAxis->range().lower;

    _pGuiModel->setStartMarkerPos(getClosestPoint(startPos));
    _pGuiModel->setEndMarkerPos(getClosestPoint(endPos));
}

void GraphView::mousePress(QMouseEvent *event)
{
    if (_pGraphViewZoom->handleMousePress(event))
    {
        /* Already handled by zoom */
    }
    else if (event->modifiers() & Qt::ControlModifier)
    {
        /* Disable range drag when control key is pressed */
        _pPlot->setInteraction(QCP::iRangeDrag, false);
        _pPlot->setInteraction(QCP::iRangeZoom, false);

        if (_pPlot->graphCount() > 0)
        {
            const double xPos = _pPlot->xAxis->pixelToCoord(event->pos().x());
            double markerPos = getClosestPoint(xPos);

            if (event->button() & Qt::LeftButton)
            {
                _pGuiModel->setStartMarkerPos(markerPos);
            }
            else if (event->button() & Qt::RightButton)
            {
                _pGuiModel->setEndMarkerPos(markerPos);
            }
            else
            {
                // No function
            }
        }
    }
    else
    {

        if (_pNoteHandling->handleMousePress(event))
        {
            /* Ignore global drag */
            /* Disable range drag when note item is selected */
            _pPlot->setInteraction(QCP::iRangeDrag, false);
            _pPlot->setInteraction(QCP::iRangeZoom, false);
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
}

void GraphView::mouseRelease(QMouseEvent *event)
{
    Q_UNUSED(event);

    (void)_pGraphViewZoom->handleMouseRelease();
    (void)_pNoteHandling->handleMouseRelease();

    /* Always re-enable range drag */
    _pPlot->setInteraction(QCP::iRangeDrag, true);
    _pPlot->setInteraction(QCP::iRangeZoom, true);
}

void GraphView::mouseWheel()
{
    if (_pGraphViewZoom->handleMouseWheel())
    {
        /* Already handled by zoom */
    }
    else
    {
        // if an axis is selected, only allow the direction of that axis to be zoomed
        // if no axis is selected, both directions may be zoomed

        if (_pPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        {
           _pPlot->axisRect()->setRangeZoom(_pPlot->xAxis->orientation());
           _pGuiModel->setxAxisScale(AxisMode::SCALE_MANUAL); // change to manual scaling
        }
        else if (_pPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        {
           _pPlot->axisRect()->setRangeZoom(_pPlot->yAxis->orientation());
           _pGuiModel->setyAxisScale(AxisMode::SCALE_MANUAL); // change to manual scaling
        }
        else
        {
           _pPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
           _pGuiModel->setyAxisScale(AxisMode::SCALE_MANUAL); // change to manual scaling
           _pGuiModel->setxAxisScale(AxisMode::SCALE_MANUAL);
        }
    }
}

void GraphView::mouseMove(QMouseEvent *event)
{
    // Check for graph drag
    if(event->buttons() & Qt::LeftButton)
    {
        if (!(event->modifiers() & Qt::ControlModifier))
        {
            if (_pGraphViewZoom->handleMouseMove(event))
            {
                /* Already handled by graph zoom */
            }
            else if (_pNoteHandling->handleMouseMove(event))
            {
                /* Already handled by graph zoom */
            }
            else
            {
                if (_pPlot->axisRect()->rangeDrag() == Qt::Horizontal)
                {
                    _pGuiModel->setxAxisScale(AxisMode::SCALE_MANUAL); // change to manual scaling
                }
                else if (_pPlot->axisRect()->rangeDrag() == Qt::Vertical)
                {
                    _pGuiModel->setyAxisScale(AxisMode::SCALE_MANUAL); // change to manual scaling
                }
                else
                {
                    // Both change to manual scaling
                    _pGuiModel->setxAxisScale(AxisMode::SCALE_MANUAL);
                    _pGuiModel->setyAxisScale(AxisMode::SCALE_MANUAL);
                }
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

void GraphView::paintTimeStampToolTip(QPoint pos)
{

    if  (_pGuiModel->cursorValues() && (_pPlot->graphCount() > 0))
    {

        const double xPos = _pPlot->xAxis->pixelToCoord(pos.x());
        double tooltipPos = getClosestPoint(xPos);

        bool bValid;
        const QCPRange keyRange = _pPlot->graph(0)->data()->keyRange(bValid);

        if (bValid && keyRange.contains(xPos))
        {
            // Add tick key string
            QString toolText = Util::formatTime(tooltipPos, false);
            QPoint location= _pPlot->mapToGlobal(pos);

            if (location != _tooltipLocation)
            {
                QToolTip::showText(location, toolText, _pPlot);

                _tooltipLocation = location;
            }
        }
        else
        {
            // Hide tooltip
            QToolTip::hideText();
            _tooltipLocation = QPoint(-1 ,-1);
        }
    }
    else
    {
        if (QToolTip::isVisible())
        {
            QToolTip::hideText();
            _tooltipLocation = QPoint(-1 ,-1);
        }
    }
}

void GraphView::handleSamplePoints()
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

void GraphView::axisDoubleClicked(QCPAxis * axis)
{
    if (axis == _pPlot->xAxis)
    {
        _pGuiModel->setxAxisScale(AxisMode::SCALE_AUTO);
    }
    else if (axis == _pPlot->yAxis)
    {
        _pGuiModel->setyAxisScale(AxisMode::SCALE_AUTO);
    }
    else
    {
        // do nothing
    }
}

void GraphView::updateData(QList<double> *pTimeData, QList<QList<double> > * pDataLists)
{
    quint64 totalPoints = 0;
    const QVector<double> timeData = pTimeData->toVector();

    for (qint32 i = 0; i < pDataLists->size(); i++)
    {
        //Add data to graphs
        QVector<double> graphData = pDataLists->at(i).toVector();
        _pPlot->graph(i)->setData(timeData, graphData, true);

        totalPoints += graphData.size();
    }

    // Check if optimizations are needed
    if (totalPoints > _cOptimizeThreshold)
    {
        _pGuiModel->setHighlightSamples(false);

        // Set width to 1
        for (qint32 i = 0; i <  _pPlot->graphCount(); i++)
        {
             _pPlot->graph(i)->pen().setWidth(1);
        }

        // Disable anti aliasing
        _pPlot->setNotAntialiasedElements(QCP::aeAll);
    }

    _pPlot->rescaleAxes(true);
    _pPlot->replot();
}

void GraphView::xAxisRangeChanged(const QCPRange &newRange, const QCPRange &oldRange)
{
    QCPRange range = newRange;

    if (newRange.upper <= 0)
    {
        range.upper = oldRange.upper;
    }

    if (newRange.lower <= 0)
    {
        range.lower = 0;
    }

    _pPlot->xAxis->setRange(range);
}

void GraphView::highlightSamples(bool bState)
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

qint32 GraphView::graphIndex(QCPGraph * pGraph)
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

double GraphView::getClosestPoint(double coordinate)
{
    QCPGraphDataContainer::const_iterator closestIt = _pPlot->graph(0)->data()->constBegin();
    QCPGraphDataContainer::const_iterator leftIt = _pPlot->graph(0)->data()->findBegin(coordinate);

    auto rightIt = leftIt + 1;
    if (rightIt !=  _pPlot->graph(0)->data()->constEnd())
    {

        const double diffReference = rightIt->key - leftIt->key;
        const double diffPos = coordinate - leftIt->key;

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

    return closestIt->key;
}
