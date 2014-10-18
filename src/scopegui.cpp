#include "scopegui.h"

#include "qcustomplot.h"

const QList<QColor> ScopeGui::_colorlist = QList<QColor>() << QColor("red")
                                                           << QColor("blue")
                                                           << QColor("gray")
                                                           << QColor("green")
                                                           << QColor("black")
                                                           << QColor("purple")
                                                           ;

   ScopeGui::ScopeGui(QCustomPlot * pPlot, QObject *parent) :
       QObject(parent)
   {

       _pPlot = pPlot;

       _pPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);

       _pPlot->xAxis->setRange(0, 10);
       _pPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
       _pPlot->xAxis->setDateTimeFormat("hh:mm:ss:zzz");
       _pPlot->xAxis->setLabel("x Axis");

       _pPlot->yAxis->setRange(0, 10);
       _pPlot->yAxis->setLabel("y Axis");

       _pPlot->plotLayout()->insertRow(0);
       _pPlot->plotLayout()->addElement(0, 0, new QCPPlotTitle(_pPlot, "Interaction Example"));

       _pPlot->legend->setVisible(true);
       QFont legendFont = QApplication::font();
       legendFont.setPointSize(10);
       _pPlot->legend->setFont(legendFont);

       // connect slot that ties some axis selections together (especially opposite axes):
       connect(_pPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
       // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
       connect(_pPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
       connect(_pPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
   }


   void ScopeGui::resetGraph(void)
   {
       _pPlot->clearGraphs();
   }

   void ScopeGui::addGraph(quint16 registerAddress)
   {
       const quint32 colorIndex = _pPlot->graphCount() % _colorlist.size();
       QCPGraph * pGraph = _pPlot->addGraph();

       QString label = QString("Register %1").arg(registerAddress);
       pGraph->setName(label);

       pGraph->setPen(QPen(_colorlist[colorIndex]));
   }

   void ScopeGui::plotResults(bool bSuccess, QList<quint16> values)
   {
       double now = QDateTime::currentDateTime().toTime_t();
       /* TODO: handle failure correctly */

       if (bSuccess)
       {
           for (qint32 i = 0; i < values.size(); i++)
           {

               _pPlot->graph(i)->addData(now, (double)values[i]);
           }

           _pPlot->replot();

           if (_settings.bXAxisAutoScale)
           {
               _pPlot->xAxis->rescale();
           }
           if (_settings.bYAxisAutoScale)
           {
               _pPlot->yAxis->rescale();
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

   void ScopeGui::mousePress()
   {
       // if an axis is selected, only allow the direction of that axis to be dragged
       // if no axis is selected, both directions may be dragged

       if (_pPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
           _pPlot->axisRect()->setRangeDrag(_pPlot->xAxis->orientation());
       else if (_pPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
           _pPlot->axisRect()->setRangeDrag(_pPlot->yAxis->orientation());
       else
           _pPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
   }

   void ScopeGui::mouseWheel()
   {
       // if an axis is selected, only allow the direction of that axis to be zoomed
       // if no axis is selected, both directions may be zoomed

       if (_pPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
           _pPlot->axisRect()->setRangeZoom(_pPlot->xAxis->orientation());
       else if (_pPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
           _pPlot->axisRect()->setRangeZoom(_pPlot->yAxis->orientation());
       else
           _pPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
   }
