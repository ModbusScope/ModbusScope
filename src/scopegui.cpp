#include "scopegui.h"

#include "qcustomplot.h"

ScopeGui::ScopeGui(QCustomPlot * pGraph, QObject *parent) :
    QObject(parent)
{
    _pGraph = pGraph;

    _pGraph->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);

    _pGraph->xAxis->setRange(0, 10);
    _pGraph->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    _pGraph->xAxis->setDateTimeFormat("hh:mm:ss:zzz");
    _pGraph->xAxis->setLabel("x Axis");

    _pGraph->yAxis->setRange(0, 10);
    _pGraph->yAxis->setLabel("y Axis");

    _pGraph->plotLayout()->insertRow(0);
    _pGraph->plotLayout()->addElement(0, 0, new QCPPlotTitle(_pGraph, "Interaction Example"));

    _pGraph->legend->setVisible(true);
    QFont legendFont = QApplication::font();
    legendFont.setPointSize(10);
    _pGraph->legend->setFont(legendFont);

    // connect slot that ties some axis selections together (especially opposite axes):
    connect(_pGraph, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(_pGraph, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(_pGraph, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
}


void ScopeGui::resetGraph(quint32 variableCount)
{
    _pGraph->clearGraphs();

    // Add graph(s)
    for (quint32 i = 0; i < variableCount; i++)
    {
        _pGraph->addGraph();
    }
}

void ScopeGui::plotResults(bool bSuccess, QList<quint16> values)
{
    double now = QDateTime::currentDateTime().toTime_t();
    /* TODO: handle failure correctly */

    if (bSuccess)
    {
        for (qint32 i = 0; i < values.size(); i++)
        {

            _pGraph->graph(i)->addData(now, (double)values[i]);
        }

        _pGraph->replot();

        if (_settings.bXAxisAutoScale)
        {
            _pGraph->xAxis->rescale();
        }
        if (_settings.bYAxisAutoScale)
        {
            _pGraph->yAxis->rescale();
        }
    }

}

void ScopeGui::setYAxisAutoScale(int state)
{
    _settings.bYAxisAutoScale = (state ? true: false);
}

void ScopeGui::setXAxisAutoScale(int state)
{
    _settings.bXAxisAutoScale = (state ? true: false);
}


void ScopeGui::selectionChanged()
{
    /*
    normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
    the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
    and the axis base line together. However, the axis label shall be selectable individually.
    */

    // handle axis and tick labels as one selectable object:
    if (_pGraph->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || _pGraph->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) || _pGraph->xAxis->selectedParts().testFlag(QCPAxis::spAxisLabel))
    {
        _pGraph->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spAxisLabel|QCPAxis::spTickLabels);
    }
    // handle axis and tick labels as one selectable object:
    if (_pGraph->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || _pGraph->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) || _pGraph->yAxis->selectedParts().testFlag(QCPAxis::spAxisLabel))
    {
        _pGraph->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spAxisLabel|QCPAxis::spTickLabels);
    }

}

void ScopeGui::mousePress()
{
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged

    if (_pGraph->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        _pGraph->axisRect()->setRangeDrag(_pGraph->xAxis->orientation());
    else if (_pGraph->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        _pGraph->axisRect()->setRangeDrag(_pGraph->yAxis->orientation());
    else
        _pGraph->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void ScopeGui::mouseWheel()
{
    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed

    if (_pGraph->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        _pGraph->axisRect()->setRangeZoom(_pGraph->xAxis->orientation());
    else if (_pGraph->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        _pGraph->axisRect()->setRangeZoom(_pGraph->yAxis->orientation());
    else
        _pGraph->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}
