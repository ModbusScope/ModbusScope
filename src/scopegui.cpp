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

   setYAxisAutoScale(true);
   setXAxisAutoScale(true);

   _pPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);

   _pPlot->xAxis->setRange(0, 10000);
   _pPlot->xAxis->setTickLabelType(QCPAxis::ltNumber);
   _pPlot->xAxis->setNumberFormat("gb");

   _pPlot->xAxis->setAutoTicks(true);
   _pPlot->xAxis->setAutoTickLabels(false);
   connect(_pPlot->xAxis, SIGNAL(ticksRequest()), this, SLOT(generateTickLabels()));

   _pPlot->xAxis->setLabel("Time (s)");

   _pPlot->yAxis->setRange(0, 10);

   _pPlot->legend->setVisible(false);
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

   _pPlot->replot();
   _pPlot->legend->setVisible(true);

   _startTime = QDateTime::currentMSecsSinceEpoch();

}

void ScopeGui::plotResults(bool bSuccess, QList<quint16> values)
{
   const quint32 diff = QDateTime::currentMSecsSinceEpoch() - _startTime;

   /* TODO: handle failure correctly */
   if (bSuccess)
   {
       for (qint32 i = 0; i < values.size(); i++)
       {
           _pPlot->graph(i)->addData(diff, (double)values[i]);
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

void ScopeGui::exportDataCsv(QString dataFile)
{

    if (_pPlot->graphCount() != 0)
    {
        const QList<double> keyList = _pPlot->graph(0)->data()->keys();
        QList<QList<QCPData> > dataList;
        QString logData;
        QString line;

        line.append("ModbusScope version;");
        line.append(APP_VERSION);
        line.append("\n");
        logData.append(line);

        QDateTime dt;
        dt = QDateTime::fromMSecsSinceEpoch(_startTime);
        line.clear();
        line.append("Start time;" + dt.toString("dd-MM-yyyy HH:mm:ss") + "\n");
        logData.append(line);

        logData.append("\n");

        line.clear();
        line.append("Time (ms)");
        for(qint32 i = 0; i < _pPlot->graphCount(); i++)
        {
            // Get headers
            line.append(";" +_pPlot->graph(i)->name());

            // Save data lists
            dataList.append(_pPlot->graph(i)->data()->values());
        }
        line.append("\n");

        logData.append(line);

        for(qint32 i = 0; i < keyList.size(); i++)
        {
            line.clear();
            line.append(QString::number(keyList[i]));

            for(qint32 d = 0; d < dataList.size(); d++)
            {
                line.append(";" + QString::number((dataList[d])[i].value));
            }
            line.append("\n");

            logData.append(line);
        }

        writeToFile(dataFile, logData);
    }
}

void ScopeGui::generateTickLabels()
{
    QVector<double> ticks = _pPlot->xAxis->tickVector();
    quint32 scaleFactor;

    /* Clear ticks vector */
    tickLabels.clear();

    /* Check if we need seconds, minute or hours on x-axis */
    if (ticks[ticks.size()-1] > _cHourTripPoint)
    {
        _pPlot->xAxis->setLabel("Time (hour)");
        scaleFactor = 60*60*1000;
    }
    else if (ticks[ticks.size()-1] > _cMinuteTripPoint)
    {
        _pPlot->xAxis->setLabel("Time (min)");
        scaleFactor = 60*1000;
    }
    else
    {
        _pPlot->xAxis->setLabel("Time (s)");
        scaleFactor = 1000;
    }

    /* Generate correct labels */
    for (qint32 index = 0; index < ticks.size(); index++)
    {
        tickLabels.append(QString::number(ticks[index] / scaleFactor));
    }

    /* Set labels */
    _pPlot->xAxis->setTickVectorLabels(tickLabels);
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



void ScopeGui::writeToFile(QString filePath, QString logData)
{
    QFile file(filePath);
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream(&file);
        stream << logData;
    }
}
