
#include <QMessageBox>

#include "scopegui.h"
#include "qcustomplot.h"

const QList<QColor> ScopeGui::_colorlist = QList<QColor>() << QColor("blue")
                                                           << QColor("gray")
                                                           << QColor("green")
                                                           << QColor("black")
                                                           << QColor("purple")
                                                           ;

ScopeGui::ScopeGui(MainWindow *window, QCustomPlot * pPlot, QObject *parent) :
   QObject(parent)
{

   _pPlot = pPlot;
   _window = window;

   _pPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);

   _pPlot->xAxis->setTickLabelType(QCPAxis::ltNumber);
   _pPlot->xAxis->setNumberFormat("gb");
   _pPlot->xAxis->setRange(0, 10000);
   _pPlot->xAxis->setAutoTicks(true);
   _pPlot->xAxis->setAutoTickLabels(false);
   _pPlot->xAxis->setLabel("Time (s)");

   connect(_pPlot->xAxis, SIGNAL(ticksRequest()), this, SLOT(generateTickLabels()));

   _settings.yMin = 0;
   _settings.yMax = 10;
   _pPlot->yAxis->setRange(_settings.yMin, _settings.yMax);

   _pPlot->legend->setVisible(false);
   QFont legendFont = QApplication::font();
   legendFont.setPointSize(10);
   _pPlot->legend->setFont(legendFont);

   // connect slot that ties some axis selections together (especially opposite axes):
   connect(_pPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

   // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
   connect(_pPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
   connect(_pPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
   connect(_pPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMove(QMouseEvent*)));

   connect(this, SIGNAL(updateXScalingUi(int)), _window,SLOT(changeXAxisScaling(int)));
   connect(this, SIGNAL(updateYScalingUi(int)), _window,SLOT(changeYAxisScaling(int)));
}


void ScopeGui::resetGraph(void)
{
   _pPlot->clearGraphs();
   graphNames.clear();
}

void ScopeGui::addGraph(quint16 registerAddress)
{
   const quint32 colorIndex = _pPlot->graphCount() % _colorlist.size();
   QCPGraph * pGraph = _pPlot->addGraph();

   QString label = QString("Register %1").arg(registerAddress);
   pGraph->setName(label);
   graphNames.append(label);

   pGraph->setPen(QPen(_colorlist[colorIndex]));

   _pPlot->replot();
   _pPlot->legend->setVisible(true);

   _startTime = QDateTime::currentMSecsSinceEpoch();

}

void ScopeGui::setxAxisScale(AxisScaleOptions scaleMode)
{
    _settings.scaleXSetting = scaleMode;
    scalePlot();
}

void ScopeGui::setxAxisScale(AxisScaleOptions scaleMode, quint32 interval)
{
    _settings.scaleXSetting = scaleMode;
    _settings.xslidingInterval = interval * 1000;
    scalePlot();
}

void ScopeGui::setyAxisScale(AxisScaleOptions scaleMode)
{
    _settings.scaleYSetting = scaleMode;
    scalePlot();
}


void ScopeGui::setyAxisScale(AxisScaleOptions scaleMode, qint32 min, qint32 max)
{
    _settings.scaleYSetting = scaleMode;
    _settings.yMin = min;
    _settings.yMax = max;
    scalePlot();
}

void ScopeGui::plotResults(QList<bool> successList, QList<quint16> valueList)
{
    const quint32 diff = QDateTime::currentMSecsSinceEpoch() - _startTime;

    for (qint32 i = 0; i < valueList.size(); i++)
    {

        if (successList[i])
        {
            // No error, add points
            _pPlot->graph(i)->addData(diff, (double)valueList[i]);

            _pPlot->graph(i)->setName(QString("(%1) %2").arg(QString::number(valueList[i])).arg(graphNames[i]));

        }
        else
        {
            _pPlot->graph(i)->addData(diff, 0);

            _pPlot->graph(i)->setName(QString("(-) %1").arg(graphNames[i]));
        }
    }

   scalePlot();

}

void ScopeGui::setxAxisSlidingInterval(int interval)
{
    _settings.xslidingInterval = (quint32)interval * 1000;
    updateXScalingUi(ScopeGui::SCALE_SLIDING); // set sliding window scaling
}

void ScopeGui::setyAxisMinMax(quint32 min, quint32 max)
{
    _settings.yMin = min;
    _settings.yMax = max;
    updateYScalingUi(ScopeGui::SCALE_MINMAX); // set min max scaling
}

qint32 ScopeGui::getyAxisMin()
{
    return _settings.yMin;
}

qint32 ScopeGui::getyAxisMax()
{
    return _settings.yMax;
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

void ScopeGui::exportGraphImage(QString imageFile)
{
    if (!_pPlot->savePng(imageFile))
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("ModbusScope export error"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Save to png file (%1) failed").arg(imageFile));
        msgBox.exec();
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

void ScopeGui::mouseWheel()
{
   // if an axis is selected, only allow the direction of that axis to be zoomed
   // if no axis is selected, both directions may be zoomed

   if (_pPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
   {
       _pPlot->axisRect()->setRangeZoom(_pPlot->xAxis->orientation());
       emit updateXScalingUi(ScopeGui::SCALE_MANUAL); // change to manual scaling
   }
   else if (_pPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
   {
       _pPlot->axisRect()->setRangeZoom(_pPlot->yAxis->orientation());
       emit updateYScalingUi(ScopeGui::SCALE_MANUAL); // change to manual scaling
   }
   else
   {
       _pPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
       emit updateXScalingUi(ScopeGui::SCALE_MANUAL); // change to manual scaling
       emit updateYScalingUi(ScopeGui::SCALE_MANUAL); // change to manual scaling
   }
}


void ScopeGui::mouseMove(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        emit updateXScalingUi(ScopeGui::SCALE_MANUAL); // change to manual scaling
        emit updateYScalingUi(ScopeGui::SCALE_MANUAL); // change to manual scaling
    }
}


void ScopeGui::writeToFile(QString filePath, QString logData)
{
    QFile file(filePath);
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream(&file);
        stream << logData;
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("ModbusScope export error"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Save to data file (%1) failed").arg(filePath));
        msgBox.exec();
    }
}

void ScopeGui::scalePlot()
{

    // scale x-axis
    if (_settings.scaleXSetting == SCALE_AUTO)
    {
        if ((_pPlot->graphCount() != 0) && (_pPlot->graph(0)->data()->keys().size()))
        {
            _pPlot->xAxis->rescale();
        }
        else
        {
            _pPlot->xAxis->setRange(0, 10000);
        }
    }
    else if (_settings.scaleXSetting == SCALE_SLIDING)
    {
        // sliding window scale routine
        if ((_pPlot->graphCount() != 0) && (_pPlot->graph(0)->data()->keys().size()))
        {
            const quint32 lastTime = _pPlot->graph(0)->data()->keys().last();
            if (lastTime > _settings.xslidingInterval)
            {
                _pPlot->xAxis->setRange(lastTime - _settings.xslidingInterval, lastTime);
            }
            else
            {
                _pPlot->xAxis->setRange(0, _settings.xslidingInterval);
            }
        }
        else
        {
            _pPlot->xAxis->setRange(0, _settings.xslidingInterval);
        }
    }
    else // Manual
    {

    }

    // scale y-axis
    if (_settings.scaleYSetting == SCALE_AUTO)
    {
        if ((_pPlot->graphCount() != 0) && (_pPlot->graph(0)->data()->keys().size()))
        {
            _pPlot->yAxis->rescale();
        }
        else
        {
            _pPlot->yAxis->setRange(0, 10);
        }
    }
    else if (_settings.scaleYSetting == SCALE_MINMAX)
    {
        // min max scale routine
        _pPlot->yAxis->setRange(_settings.yMin, _settings.yMax);
    }
    else // Manual
    {

    }

    _pPlot->replot();
}
