
#include "graphscaling.h"
#include "guimodel.h"
#include "graphview.h"

#include "axistickertime.h"
#include "valueaxis.h"

GraphScale::GraphScale(GuiModel* pGuiModel, ScopePlot* pPlot, QObject *parent) :
    QObject(parent),
    _pGuiModel(pGuiModel),
    _pPlot(pPlot)
{
    _pGraphview = dynamic_cast<GraphView*>(parent);

    /* Range drag is also enabled/disabled on mousePress and mouseRelease event */
    _pPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);

    // disable anti aliasing while dragging
    _pPlot->setNoAntialiasingOnDrag(true);

    // Replace y-axis with custom value axis
    _pPlot->axisRect()->removeAxis(_pPlot->axisRect()->axes(QCPAxis::atLeft)[0]);
    _pPlot->axisRect()->addAxis(QCPAxis::atLeft, new ValueAxis(_pPlot->axisRect(), QCPAxis::atLeft));

    connect(_pPlot->xAxis, QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged), this, &GraphScale::timeAxisRangeChanged);
    
    // Fix axis settings
    QCPAxis * pXAxis = _pPlot->axisRect()->axes(QCPAxis::atBottom)[0];
    QCPAxis * pYAxis = _pPlot->axisRect()->axes(QCPAxis::atLeft)[0];
    pYAxis->grid()->setVisible(true);
    _pPlot->axisRect()->setRangeDragAxes(pXAxis, pYAxis);
    _pPlot->axisRect()->setRangeZoomAxes(pXAxis, pYAxis);

    // Add custom axis ticker
    QSharedPointer<QCPAxisTickerTime> timeTicker(new AxisTickerTime(_pPlot));
    _pPlot->xAxis->setTicker(timeTicker);
    _pPlot->xAxis->setLabel("Time");
    _pPlot->xAxis->setRange(0, 10000);

    _pPlot->yAxis->setRange(0, 65535);

    // connect slot that ties some axis selections together (especially opposite axes):
    connect(_pPlot, &ScopePlot::selectionChangedByUser, this, &GraphScale::selectionChanged);
    connect(_pPlot, &ScopePlot::axisDoubleClick, this, &GraphScale::axisDoubleClicked);
}

GraphScale::~GraphScale()
{

}

void GraphScale::rescale()
{
    // scale x-axis
    if (_pGuiModel->xAxisScalingMode() == AxisMode::SCALE_AUTO)
    {
        if ((_pPlot->graphCount() != 0) && (_pGraphview->graphDataSize() != 0))
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
        if ((_pPlot->graphCount() != 0) && (_pGraphview->graphDataSize() != 0))
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
        if ((_pPlot->graphCount() != 0) && (_pGraphview->graphDataSize()))
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
        auto pAxis = dynamic_cast<ValueAxis *>(_pPlot->yAxis);
        if (pAxis != nullptr)
        {
            pAxis->rescaleValue(_pPlot->xAxis->range());
        }
    }
    else // Manual
    {

    }
}

void GraphScale::selectionChanged()
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

void GraphScale::axisDoubleClicked(QCPAxis* axis)
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

void GraphScale::timeAxisRangeChanged(const QCPRange &newRange)
{
    double newLower = newRange.lower;
    double newUpper = newRange.upper;

    QList<QCPGraph*> const graphList = _pPlot->xAxis->graphs();

    if (graphList.size() > 0 && !graphList[0]->data()->isEmpty())
    {
        const double beginKey = graphList[0]->data()->constBegin()->key;
        if (newLower < 0)
        {
            if (beginKey > 0)
            {
                newLower = 0;
            }
            else
            {
                newLower = newLower < beginKey ? beginKey: newLower;
            }
        }

        if (newUpper < newLower)
        {
            newUpper = newLower;
        }
    }

    _pPlot->xAxis->setRange(newLower, newUpper);
}

void GraphScale::disableRangeZoom()
{
    _pPlot->setInteraction(QCP::iRangeDrag, false);
    _pPlot->setInteraction(QCP::iRangeZoom, false);
}

void GraphScale::enableRangeZoom()
{
    _pPlot->setInteraction(QCP::iRangeDrag, true);
    _pPlot->setInteraction(QCP::iRangeZoom, true);
}

void GraphScale::configureDragDirection()
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

void GraphScale::zoomGraph()
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

void GraphScale::handleDrag()
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
