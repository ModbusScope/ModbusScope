#include "graphqualitymarkers.h"

#include "models/graphdatamodel.h"

#include <array>
#include <utility>

/* The states that get a marker. Good needs none: it is the plain line already drawn. */
static constexpr std::array<DataQuality::State, 3> cMarkedStates = { DataQuality::State::Degraded,
                                                                     DataQuality::State::Invalid,
                                                                     DataQuality::State::NoValue };
static constexpr qint32 cMarkedStateCount = static_cast<qint32>(cMarkedStates.size());

static const double cMarkerSize = 8;
static const char* const cMarkerLayer = "qualityMarkers";

/*!
 * \brief Returns the position of \a state in the marked-state list, or -1 when it gets no marker.
 */
static qint32 markerIndex(DataQuality::State state)
{
    for (qint32 idx = 0; idx < cMarkedStateCount; idx++)
    {
        if (cMarkedStates.at(idx) == state)
        {
            return idx;
        }
    }

    return -1;
}

/*!
 * \brief Returns the marker path for Invalid: a diamond with an inscribed cross.
 *
 * QCPScatterStyle has no built-in "diamond with a cross" shape, only circle/square variants
 * (ssCrossCircle, ssCrossSquare), which fill the shape and then stroke a cross on top with the
 * same pen. This builds the same thing for a diamond via ssCustom.
 *
 * Coordinates follow ssCustom's reference size of 6 (its drawShape() case scales the path by
 * size/6): the diamond's half-width is 3, and the cross is inscribed at the diamond's edge
 * (|x|+|y|=3, so x=y=1.5 on the diagonal) rather than spanning the full bounding box, matching how
 * ssCrossCircle keeps its cross inside the circle instead of poking out of it.
 */
static QPainterPath invalidMarkerPath()
{
    QPainterPath path;
    path.moveTo(-3, 0);
    path.lineTo(0, -3);
    path.lineTo(3, 0);
    path.lineTo(0, 3);
    path.closeSubpath();

    path.moveTo(-1.5, -1.5);
    path.lineTo(1.5, 1.5);
    path.moveTo(-1.5, 1.5);
    path.lineTo(1.5, -1.5);

    return path;
}

/*!
 * \brief Returns the scatter style that represents \a state in the plot.
 *
 * Colors are fixed per quality state instead of following the signal color, so the same symbol
 * always means the same thing regardless of which signal it belongs to. Invalid reuses the red
 * the legend already uses for invalid values. Shapes are solid-filled (pen and brush the same
 * color) rather than outlines, so a marker fully covers the graph's own scatter dot underneath it
 * instead of leaving it visible through the middle (e.g. when "highlight sample points" is on).
 */
static QCPScatterStyle scatterStyle(DataQuality::State state)
{
    switch (state)
    {
    case DataQuality::State::Degraded:
        return QCPScatterStyle(QCPScatterStyle::ssTriangle, QColor(255, 140, 0), QColor(255, 140, 0), cMarkerSize);
    case DataQuality::State::Invalid:
    {
        static const QPainterPath cInvalidPath = invalidMarkerPath();
        return QCPScatterStyle(cInvalidPath, QPen(GraphDataModel::lightRed), QBrush(GraphDataModel::lightRed),
                               cMarkerSize);
    }
    case DataQuality::State::NoValue:
        return QCPScatterStyle(QCPScatterStyle::ssSquare, Qt::gray, Qt::gray, cMarkerSize);
    case DataQuality::State::Good:
        break;
    }

    return QCPScatterStyle(QCPScatterStyle::ssNone);
}

GraphQualityMarkers::GraphQualityMarkers(GraphDataModel* pGraphDataModel, ScopePlot* pPlot, QObject* parent)
    : QObject(parent), _pGraphDataModel(pGraphDataModel), _pPlot(pPlot)
{
    /* Keep the markers above the graph layers so they are never hidden by a graph brought to front */
    _pPlot->addLayer(QLatin1String(cMarkerLayer), _pPlot->layer("topMain"), QCustomPlot::limAbove);
}

GraphQualityMarkers::~GraphQualityMarkers()
{
    removeOverlays();
}

/*!
 * \brief Recreates every overlay curve from the data model.
 *
 * The full rebuild keeps the overlays a pure function of the model: every path that changes signal
 * data, axis assignment or visibility can simply call this instead of applying a matching partial
 * update of its own. Only the live sample path (appendSample()) updates incrementally.
 */
void GraphQualityMarkers::rebuild()
{
    removeOverlays();

    QList<GraphIdx> activeGraphList;
    _pGraphDataModel->activeGraphIndexList(activeGraphList);

    for (const GraphIdx& graphIdx : std::as_const(activeGraphList))
    {
        addOverlays(graphIdx);
        loadSeries(graphIdx);
    }
}

/*!
 * \brief Adds a single freshly polled sample to the overlay matching its quality.
 * \param graphIdx Graph index in the full graph list.
 * \param timestamp Timestamp of the sample.
 * \param value Value of the sample.
 * \param quality Data quality of the sample.
 */
void GraphQualityMarkers::appendSample(GraphIdx graphIdx,
                                       double timestamp,
                                       double value,
                                       const DataQuality::Quality& quality)
{
    const qint32 stateIdx = markerIndex(quality.state);
    if (stateIdx < 0)
    {
        return;
    }

    const auto overlayIt = _overlays.constFind(graphIdx);
    if (overlayIt == _overlays.constEnd())
    {
        return;
    }

    overlayIt.value().at(stateIdx)->addData(timestamp, value);
}

/*!
 * \brief Moves the overlays of a signal to the primary or secondary value axis.
 * \param graphIdx Graph index in the full graph list.
 * \param axis Value axis the signal is plotted against.
 */
void GraphQualityMarkers::setAxis(GraphIdx graphIdx, GraphData::valueAxis_t axis)
{
    const auto overlayIt = _overlays.constFind(graphIdx);
    if (overlayIt == _overlays.constEnd())
    {
        return;
    }

    QCPAxis* pValueAxis = (axis == GraphData::VALUE_AXIS_SECONDARY) ? _pPlot->yAxis2 : _pPlot->yAxis;
    for (QCPCurve* pCurve : overlayIt.value())
    {
        pCurve->setValueAxis(pValueAxis);
    }
}

/*!
 * \brief Shows or hides the overlays of a signal.
 * \param graphIdx Graph index in the full graph list.
 * \param bVisible Whether the signal is visible.
 */
void GraphQualityMarkers::setVisible(GraphIdx graphIdx, bool bVisible)
{
    const auto overlayIt = _overlays.constFind(graphIdx);
    if (overlayIt == _overlays.constEnd())
    {
        return;
    }

    for (QCPCurve* pCurve : overlayIt.value())
    {
        pCurve->setVisible(bVisible);
    }
}

/*!
 * \brief Removes every overlay curve from the plot.
 */
void GraphQualityMarkers::removeOverlays()
{
    for (const QList<QCPCurve*>& curveList : std::as_const(_overlays))
    {
        for (QCPCurve* pCurve : curveList)
        {
            _pPlot->removePlottable(pCurve);
        }
    }

    _overlays.clear();
}

/*!
 * \brief Creates the (empty) overlay curves of a single signal.
 * \param graphIdx Graph index in the full graph list.
 */
void GraphQualityMarkers::addOverlays(GraphIdx graphIdx)
{
    QList<QCPCurve*> curveList;

    for (qint32 idx = 0; idx < cMarkedStateCount; idx++)
    {
        QCPCurve* pCurve = new QCPCurve(_pPlot->xAxis, _pPlot->yAxis);
        pCurve->setLineStyle(QCPCurve::lsNone);
        pCurve->setScatterStyle(scatterStyle(cMarkedStates.at(idx)));
        pCurve->setSelectable(QCP::stNone);
        pCurve->setLayer(QLatin1String(cMarkerLayer));
        pCurve->removeFromLegend();

        curveList.append(pCurve);
    }

    _overlays.insert(graphIdx, curveList);

    setAxis(graphIdx, _pGraphDataModel->valueAxis(graphIdx));
    setVisible(graphIdx, _pGraphDataModel->isVisible(graphIdx));
}

/*!
 * \brief Fills the overlays of a signal with the samples of its data series.
 * \param graphIdx Graph index in the full graph list.
 */
void GraphQualityMarkers::loadSeries(GraphIdx graphIdx)
{
    const QSharedPointer<const GraphDataSeries> pDataSeries = _pGraphDataModel->dataSeries(graphIdx);

    QList<QVector<double> > timestamps;
    QList<QVector<double> > values;
    for (qint32 idx = 0; idx < cMarkedStateCount; idx++)
    {
        timestamps.append(QVector<double>());
        values.append(QVector<double>());
    }

    GraphDataSeries::const_iterator it = pDataSeries->constBegin();
    while (it != pDataSeries->constEnd())
    {
        const qint32 stateIdx = markerIndex(it->quality.state);
        if (stateIdx >= 0)
        {
            timestamps[stateIdx].append(it->timestamp);
            values[stateIdx].append(it->value);
        }
        ++it;
    }

    const QList<QCPCurve*> curveList = _overlays.value(graphIdx);
    for (qint32 idx = 0; idx < cMarkedStateCount; idx++)
    {
        curveList.at(idx)->setData(timestamps.at(idx), values.at(idx));
    }
}
