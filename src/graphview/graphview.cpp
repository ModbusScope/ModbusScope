
#include "graphview.h"

#include "graphview/graphindicators.h"
#include "graphview/graphmarkers.h"
#include "graphview/graphscaling.h"
#include "graphview/graphviewzoom.h"
#include "graphview/notehandling.h"
#include "graphview/scopeplot.h"
#include "models/graphdatamodel.h"
#include "models/guimodel.h"
#include "models/notemodel.h"
#include "models/settingsmodel.h"
#include "util/formatrelativetime.h"
#include "util/result.h"

#include <QInputDialog>
#include <QLocale>
#include <QVector>
#include <QtGlobal>

GraphView::GraphView(GuiModel * pGuiModel, SettingsModel *pSettingsModel, GraphDataModel * pGraphDataModel, NoteModel *pNoteModel, ScopePlot * pPlot, QObject *parent) :
    QObject(parent)
{
    _pGuiModel = pGuiModel;
    _pGraphDataModel = pGraphDataModel;
    _pSettingsModel = pSettingsModel;

    _pPlot = pPlot;

    /*
    * Greatly improves performance
    *
    * phFastPolylines	Graph/Curve lines are drawn with a faster method. This reduces the quality especially
    *                   of the line segment joins. (Only relevant for solid line pens.)
    * phCacheLabels		axis (tick) labels will be cached as pixmaps, increasing replot performance.
    * */
    _pPlot->setPlottingHints(QCP::phCacheLabels | QCP::phFastPolylines);

    _pPlot->setInteraction(QCP::iSelectPlottables, true);

    /*
    * Use other modifier key than Ctrl for multi select.
    * ModbusScope uses Ctrl for adding/moving marker.
    * Note: Multi select is disabled, but modifier needs to be changed from default.
    * */
    _pPlot->setMultiSelectModifier(Qt::ShiftModifier);

    // Samples are enabled
    _bEnableSampleHighlight = true;

    // Add layer to move graph in front
    _pPlot->addLayer("topMain", _pPlot->layer("main"), QCustomPlot::limAbove);

    // Add layer to move graph indicator in front (axes)
    _pPlot->addLayer("topAxes", _pPlot->layer("axes"), QCustomPlot::limAbove);

    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(_pPlot, &ScopePlot::mousePress, this, &GraphView::mousePress);
    connect(_pPlot, &ScopePlot::mouseRelease, this, &GraphView::mouseRelease);
    connect(_pPlot, &ScopePlot::mouseWheel, this, &GraphView::mouseWheel);
    connect(_pPlot, &ScopePlot::mouseMove, this, &GraphView::mouseMove);
    connect(_pPlot, &ScopePlot::beforeReplot, this, &GraphView::enableSamplePoints);

    _pGraphScale = new GraphScale(_pGuiModel, _pPlot, this);
    _pGraphViewZoom = new GraphViewZoom(_pGuiModel, _pPlot, this);
    _pGraphMarkers = new GraphMarkers(pGraphDataModel, _pGuiModel, _pPlot, this);
    _pGraphIndicators = new GraphIndicators(_pGraphDataModel, _pPlot, this);
    _pNoteHandling = new NoteHandling(pNoteModel, _pPlot, this);

    updateSecondaryAxisVisibility();

    _pPlot->replot();
}

GraphView::~GraphView()
{
    delete _pGraphScale;
    delete _pGraphViewZoom;
    delete _pGraphMarkers;
    delete _pGraphIndicators;
    delete _pNoteHandling;
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
    _pGraphMarkers->clearTracers();
    _pGraphIndicators->clear();

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
            QCPGraph * pGraph = _pPlot->addGraph();
            setGraphAxis(pGraph, _pGraphDataModel->valueAxis(graphIdx));
            setGraphColor(pGraph, _pGraphDataModel->color(graphIdx));
            pGraph->setSelectable(QCP::stWhole);
            pGraph->setVisible(_pGraphDataModel->isVisible(graphIdx));

            connect(pGraph, QOverload<bool>::of(&QCPGraph::selectionChanged), this, &GraphView::handleSelectionChanged);

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

            pGraph->setData(pMap);

            _pGraphMarkers->addTracer(pGraph);
            _pGraphIndicators->add(graphIdx, pGraph);
        }
    }

    updateSecondaryAxisVisibility();

    _pPlot->replot();

    emit afterGraphUpdate();
}

void GraphView::changeGraphColor(const quint32 graphIdx)
{
    if (_pGraphDataModel->isActive(graphIdx))
    {
        const quint32 activeIdx = static_cast<quint32>(_pGraphDataModel->convertToActiveGraphIndex(graphIdx));

        setGraphColor(_pPlot->graph(activeIdx), _pGraphDataModel->color(graphIdx));

        _pPlot->replot();
    }
}

void GraphView::changeGraphAxis(const quint32 graphIdx)
{   
    if (_pGraphDataModel->isActive(graphIdx))
    {
        const quint32 activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);
        setGraphAxis(_pPlot->graph(activeIdx), _pGraphDataModel->valueAxis(graphIdx));

        updateSecondaryAxisVisibility();

        _pPlot->replot();
    }
}

void GraphView::changeSelectedGraph(const qint32 activeGraphIdx)
{
    QList<QCPGraph*> selectedGraphs = _pPlot->selectedGraphs();

    if (activeGraphIdx == -1)
    {
        for (QCPGraph* selectedGraph : std::as_const(selectedGraphs))
        {
            selectedGraph->setSelection(QCPDataSelection());
        }
    }
    else
    {
        auto graph = _pPlot->graph(activeGraphIdx);
        const bool bAlreadySelected = selectedGraphs.contains(graph);

        if (!bAlreadySelected)
        {
            for (QCPGraph* selectedGraph : std::as_const(selectedGraphs))
            {
                selectedGraph->setSelection(QCPDataSelection());
            }

            graph->setSelection(QCPDataSelection(QCPDataRange(0, 1)));
        }

        bringToFront(activeGraphIdx);
    }

    _pPlot->replot();
}

void GraphView::bringToFront(const qint32 activeGraphIdx)
{
    _pPlot->graph(activeGraphIdx)->setLayer("topMain");

    const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(activeGraphIdx);

    _pGraphIndicators->setFrontGraph(graphIdx);
    _pPlot->replot();
}

void GraphView::addData(QList<double> timeData, QList<QList<double> > data)
{
    quint64 totalPoints = 0;
    const QVector<double> timeDataVector = timeData.toVector();

    for (qint32 i = 0; i < data.size(); i++)
    {
        QVector<double> graphData = data.at(i).toVector();
        _pPlot->graph(i)->setData(timeDataVector, graphData, true);

        totalPoints += graphData.size();
    }

    // Check if optimizations are needed
    if (totalPoints > _cOptimizeThreshold)
    {
        _pGuiModel->setHighlightSamples(false);

        for (qint32 i = 0; i <  _pPlot->graphCount(); i++)
        {
             _pPlot->graph(i)->pen().setWidth(1);
        }

        _pPlot->setNotAntialiasedElements(QCP::aeAll);
    }

    _pPlot->rescaleAxes(true);
    _pPlot->replot();
}

void GraphView::handleGraphVisibilityChange(quint32 graphIdx)
{
    if (_pGraphDataModel->isActive(graphIdx))
    {
        const bool bShow = _pGraphDataModel->isVisible(graphIdx);

        const quint32 activeIdx = static_cast<quint32>(_pGraphDataModel->convertToActiveGraphIndex(graphIdx));

        _pPlot->graph(activeIdx)->setVisible(bShow);
        _pGraphMarkers->updateTracersVisibility();
        _pGraphIndicators->updateIndicatorVisibility();
        rescalePlot();
    }
}

void GraphView::rescalePlot()
{
    _pGraphScale->rescale();

    _pPlot->replot();
}

void GraphView::plotResults(ResultDoubleList resultList)
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
        timeData = QDateTime::currentMSecsSinceEpoch() - _pGraphDataModel->communicationStartTime();
    }

    QList<double> dataList;

    uint32_t i = 0;
    for (const auto &result: resultList)
    {
        if (result.isValid())
        {
            // No error, add points
            _pPlot->graph(i)->addData(timeData, result.value());
            dataList.append(result.value());
        }
        else
        {
            _pPlot->graph(i)->addData(timeData, 0);
            dataList.append(0);
        }
        i++;
    }

    emit dataAddedToPlot(timeData, dataList);

   rescalePlot();
}

void GraphView::clearResults()
{
    for (qint32 i = 0; i < _pPlot->graphCount(); i++)
    {
        _pPlot->graph(i)->data()->clear();
    }

    rescalePlot();
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

        /* Disable range drag when zooming is active */
        _pGraphScale->disableRangeZoom();
    }
    else if (event->modifiers() & Qt::ControlModifier)
    {
        /* Disable range drag when control key is pressed */
        _pGraphScale->disableRangeZoom();

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
            _pGraphScale->disableRangeZoom();
        }
        else
        {
            _pGraphScale->configureDragDirection();
        }
    }
}

void GraphView::mouseRelease(QMouseEvent *event)
{
    Q_UNUSED(event);

    (void)_pGraphViewZoom->handleMouseRelease();
    (void)_pNoteHandling->handleMouseRelease();

    /* Always re-enable range drag */
    _pGraphScale->enableRangeZoom();
}

void GraphView::mouseWheel()
{
    if (_pGraphViewZoom->handleMouseWheel())
    {
        /* Already handled by zoom */
    }
    else
    {
        _pGraphScale->zoomGraph();
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
                _pGraphScale->handleDrag();
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

void GraphView::handleSelectionChanged(bool selected)
{
    if (selected)
    {
        QCPGraph * pGraph = qobject_cast<QCPGraph *>(QObject::sender());
        const qint32 activeIdx = getActiveGraphIndex(pGraph);
        const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(activeIdx);
        _pGraphDataModel->setSelectedGraph(graphIdx);
    }
    else
    {
        _pGraphDataModel->setSelectedGraph(-1);
    }

    for (qint32 idx = 0; idx < _pPlot->graphCount(); idx++)
    {
        QPen graphPen = _pPlot->graph(idx)->pen();
        QColor baseColor = graphPen.color();
        QColor normalPen = QColor(baseColor.red(), baseColor.green(), baseColor.blue(), 255);
        QColor transparentPen = QColor(baseColor.red(), baseColor.green(), baseColor.blue(), 16);

        if ((_pGraphDataModel->selectedGraph() == -1) || (idx == _pGraphDataModel->selectedGraph()))
        {
            graphPen.setColor(normalPen);
        }
        else
        {
            graphPen.setColor(transparentPen);
        }

        _pPlot->graph(idx)->setPen(graphPen);
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
            QString toolText = FormatRelativeTime::formatTime(tooltipPos);
            QPoint location= _pPlot->mapToGlobal(pos);

            if (location != _tooltipLocation)
            {
                QToolTip::showText(location, toolText, _pPlot);

                _tooltipLocation = location;
            }
        }
        else
        {
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

void GraphView::highlightSamples(bool bState)
{
    for (qint32 graphIndex = 0; graphIndex < _pPlot->graphCount(); graphIndex++)
    {
        if (bState)
        {
            _pPlot->graph(graphIndex)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 3));
        }
        else
        {
            _pPlot->graph(graphIndex)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone));
        }
    }
}

void GraphView::setGraphColor(QCPGraph* _pGraph, const QColor &color)
{
    QPen pen;
    pen.setColor(color);
    pen.setWidth(2);
    pen.setCosmetic(true);
    _pGraph->setPen(pen);
    _pGraph->selectionDecorator()->setPen(pen);
}

void GraphView::setGraphAxis(QCPGraph* _pGraph, const GraphData::valueAxis_t &axis)
{
    if (axis == GraphData::VALUE_AXIS_SECONDARY)
    {
        _pGraph->setValueAxis(_pPlot->yAxis2);
    }
    else
    {
        _pGraph->setValueAxis(_pPlot->yAxis);
    }
}

double GraphView::getClosestPoint(double coordinate)
{
    if ((_pPlot->graphCount() > 0) && (graphDataSize() != 0))
    {
        QCPGraphDataContainer::const_iterator closestIt;
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
    else
    {
        return coordinate;
    }
}

void GraphView::updateSecondaryAxisVisibility()
{
    bool bSecondaryVisibility = false;
    for (qint32 activeGraphIndex = 0; activeGraphIndex < _pPlot->graphCount(); activeGraphIndex++)
    {
        const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(activeGraphIndex);
        if (_pGraphDataModel->valueAxis(graphIdx) == GraphData::VALUE_AXIS_SECONDARY)
        {
            bSecondaryVisibility = true;
            break;
        }
    }

    _pPlot->yAxis2->setVisible(bSecondaryVisibility);

    rescalePlot();
}

qint32 GraphView::getActiveGraphIndex(QCPGraph const * const pGraph)
{
    qint32 idx = -1;
    for (int i = 0; i < _pPlot->graphCount(); ++i)
    {
        if (_pPlot->graph(i) == pGraph)
        {
            idx = i;
            break;
        }
    }
    return idx;
}
