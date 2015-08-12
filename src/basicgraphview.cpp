#include <QMessageBox>
#include <QVector>
#include <QtGlobal>
#include <QLocale>

#include <algorithm> // std::upperbound, std::lowerbound

#include "guimodel.h"
#include "basicgraphview.h"

BasicGraphView::BasicGraphView(GuiModel * pGuiModel, QCustomPlot * pPlot, QObject *parent) :
    QObject(parent)
{
    _pGuiModel = pGuiModel;

   _pPlot = pPlot;

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

   _pPlot->legend->setVisible(false);
   QFont legendFont = QApplication::font();
   legendFont.setPointSize(10);
   _pPlot->legend->setFont(legendFont);

   // Tooltip is enabled
   _bEnableTooltip = true;

   // Samples are enabled
   _bEnableSampleHighlight = true;

   // Add layer to move graph on front
   _pPlot->addLayer("topMain", _pPlot->layer("main"), QCustomPlot::limAbove);

   // connect slot that ties some axis selections together (especially opposite axes):
   connect(_pPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

   // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
   connect(_pPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
   connect(_pPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
   connect(_pPlot, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(axisDoubleClicked(QCPAxis*)));
   connect(_pPlot, SIGNAL(legendClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), this, SLOT(legendClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)));
   connect(_pPlot, SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), this, SLOT(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)));
   connect(_pPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(paintValueToolTip(QMouseEvent*)));
   connect(_pPlot, SIGNAL(beforeReplot()), this, SLOT(handleSamplePoints()));
}

BasicGraphView::~BasicGraphView()
{

}

void BasicGraphView::exportGraphImage(QString imageFile)
{
    if (!_pPlot->savePng(imageFile))
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Export image"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Save to png file (%1) failed").arg(imageFile));
        msgBox.exec();
    }
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
    _pPlot->rescaleAxes(true);
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

void BasicGraphView::clearGraphs()
{
    _pPlot->clearGraphs();
    _pPlot->replot();
}

void BasicGraphView::addGraphs()
{
    for(quint32 idx = 0; idx < _pGuiModel->graphCount(); idx++)
    {
        QCPGraph * pGraph = _pPlot->addGraph();

        pGraph->setName(_pGuiModel->graphLabel(idx));

        QPen pen;
        pen.setColor(_pGuiModel->graphColor(idx));
        pen.setWidth(2);
        pen.setCosmetic(true);

        pGraph->setPen(pen);
    }

    _pPlot->replot();
}

void BasicGraphView::showHideLegend()
{
    _pPlot->legend->setVisible(_pGuiModel->legendVisibility());
    _pPlot->replot();
}

void BasicGraphView::showGraph(quint32 index)
{
    const bool bShow = _pGuiModel->graphVisibility(index);
    _pPlot->graph(index)->setVisible(bShow);

    QFont itemFont = _pPlot->legend->item(index)->font();
    itemFont.setStrikeOut(!bShow);

    _pPlot->legend->item(index)->setFont(itemFont);

    _pPlot->replot();
}

void BasicGraphView::bringToFront()
{
    if (_pPlot->graphCount() > 0)
    {
        _pPlot->graph(_pGuiModel->frontGraph())->setLayer("topMain");
        _pPlot->replot();
    }
}

void BasicGraphView::updateLegendPosition()
{
    if (_pGuiModel->legendPosition() == LEGEND_LEFT)
    {
         _pPlot ->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft|Qt::AlignTop);
    }
    else if (_pGuiModel->legendPosition() == LEGEND_MIDDLE)
    {
         _pPlot ->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignCenter|Qt::AlignTop);
    }
    else if (_pGuiModel->legendPosition() == LEGEND_RIGHT)
    {
         _pPlot ->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignTop);
    }
    _pPlot->replot();
}

void BasicGraphView::generateTickLabels()
{
    QVector<double> ticks = _pPlot->xAxis->tickVector();

    /* Clear ticks vector */
    tickLabels.clear();

    /* Generate correct labels */
    for (qint32 index = 0; index < ticks.size(); index++)
    {
        tickLabels.append(createTickLabelString(ticks[index]));
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

void BasicGraphView::mousePress()
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

void BasicGraphView::legendClick(QCPLegend * legend, QCPAbstractLegendItem * abstractLegendItem, QMouseEvent * event)
{
    Q_UNUSED(event);

    if ((NULL != legend) && (NULL != abstractLegendItem))
    {
        // Check for selection
        QCPPlottableLegendItem *legendItem = qobject_cast<QCPPlottableLegendItem*>(abstractLegendItem);
        if (legendItem != 0)
        {
            const qint32 graphIndex = this->graphIndex(qobject_cast<QCPGraph*>(legendItem->plottable()));
            if (graphIndex >= 0)
            {
                _pGuiModel->setFrontGraph(graphIndex);
            }
        }
    }
}

void BasicGraphView::legendDoubleClick(QCPLegend * legend,QCPAbstractLegendItem * abstractLegendItem, QMouseEvent * event)
{
    Q_UNUSED(event);

    if ((NULL != legend) && (NULL != abstractLegendItem))
    {
        // Check for selection
        QCPPlottableLegendItem *legendItem = qobject_cast<QCPPlottableLegendItem*>(abstractLegendItem);
        if (legendItem != 0)
        {
            const qint32 graphIndex = this->graphIndex(qobject_cast<QCPGraph*>(legendItem->plottable()));
            if (graphIndex >= 0)
            {
                _pGuiModel->setGraphVisibility(graphIndex, !_pGuiModel->graphVisibility(graphIndex));
            }
        }
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

                        // Add tick key string
                        toolText = createTickLabelString(keyList[keyIndex]);

                        // Check all graphs
                        for (qint32 graphIndex = 0; graphIndex < _pPlot->graphCount(); graphIndex++)
                        {
                            if (_pPlot->graph(graphIndex)->visible())
                            {
                                const double value = _pPlot->graph(graphIndex)->data()->values()[keyIndex].value;
                                toolText += QString("\n%1: %2").arg(_pGuiModel->graphLabel(graphIndex)).arg(value);
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

QString BasicGraphView::createTickLabelString(qint64 tickKey)
{
    QString tickLabel;
    bool bNegative;
    quint64 tmp;

    if (tickKey < 0)
    {
        bNegative = true;
        tmp = -1 * tickKey;
    }
    else
    {
        bNegative = false;
        tmp = tickKey;
    }

    tmp %= 24 * 60 * 60 * 1000; // Number of seconds in a day

    quint32 hours = tmp / (60 * 60 * 1000);
    tmp = tmp % (60 * 60 * 1000);

    quint32 minutes = tmp / (60 * 1000);
    tmp = tmp % (60 * 1000);

    quint32 seconds = tmp / 1000;
    quint32 milliseconds = tmp % 1000;

    tickLabel = QString("%1:%2:%3%4%5").arg(hours)
                                                .arg(minutes, 2, 10, QChar('0'))
                                                .arg(seconds, 2, 10, QChar('0'))
                                                .arg(QLocale::system().decimalPoint())
                                               .arg(milliseconds, 2, 10, QChar('0'));

    // Make sure minus sign is shown when tick number is negative
    if (bNegative)
    {
        tickLabel = "-" + tickLabel;
    }

    return tickLabel;
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
