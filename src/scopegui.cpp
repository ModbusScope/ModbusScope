
#include <QMessageBox>
#include <QVector>
//#include <QLocale>

#include "util.h"
#include "scopegui.h"
#include "qcustomplot.h"


const QList<QColor> ScopeGui::_colorlist = QList<QColor>() << QColor("blue")
                                                           << QColor("green")
                                                           << QColor("red")
                                                           << QColor("yellow")
                                                           << QColor("orange")
                                                           << QColor("purple")
                                                           << QColor("black")
                                                           ;

ScopeGui::ScopeGui(MainWindow *window, ScopeData *scopedata, QCustomPlot * pPlot, QObject *parent) :
   QObject(parent)
{

   _pPlot = pPlot;
   _window = window;
   _scopedata = scopedata;

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

   _settings.yMin = 0;
   _settings.yMax = 10;
   _pPlot->yAxis->setRange(_settings.yMin, _settings.yMax);

   _pPlot->legend->setVisible(false);
   QFont legendFont = QApplication::font();
   legendFont.setPointSize(10);
   _pPlot->legend->setFont(legendFont);

   // Add layer to move graph on front
   _pPlot->addLayer("topMain", _pPlot->layer("main"), QCustomPlot::limAbove);

   // connect slot that ties some axis selections together (especially opposite axes):
   connect(_pPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

   // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
   connect(_pPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
   connect(_pPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
   connect(_pPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMove(QMouseEvent*)));
   connect(_pPlot, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(axisDoubleClicked(QCPAxis*)));

   connect(this, SIGNAL(updateXScalingUi(int)), _window,SLOT(changeXAxisScaling(int)));
   connect(this, SIGNAL(updateYScalingUi(int)), _window,SLOT(changeYAxisScaling(int)));
}


void ScopeGui::resetGraph(void)
{
    _pPlot->clearGraphs();
    _graphNames.clear();
}

void ScopeGui::setupGraph(QList<QString> registerTextList)
{
   quint32 colorIndex;

   for (qint32 i = 0; i < registerTextList.size(); i++)
   {
       colorIndex = _pPlot->graphCount() % _colorlist.size();

       QCPGraph * pGraph = _pPlot->addGraph();

       pGraph->setName(registerTextList[i]);
       _graphNames.append(registerTextList[i]);

       QPen pen;
       pen.setColor(_colorlist[colorIndex]);
       pen.setWidth(2);
       pen.setCosmetic(true);

       pGraph->setPen(pen);
   }

   _pPlot->replot();
   _pPlot->legend->setVisible(true);

}

void ScopeGui::loadFileData(DataFileParser::FileData * pData)
{
    // Clear data
    resetGraph();

    // Init graphs, remove first label because is the time scale
    QStringList labelList = QStringList(pData->dataLabel);
    labelList.removeFirst();
    setupGraph(labelList);

    QVector<double> timeData = pData->dataRows.at(0).toVector();

    //Add data to graphs
    for (qint32 i = 1; i < pData->dataLabel.size(); i++)
    {
        QVector<double> graphData = pData->dataRows.at(i).toVector();
        _pPlot->graph(i-1)->addData(timeData, graphData);
    }

    //Rescale
    scalePlot();

}

void ScopeGui::setxAxisScale(AxisScaleOptions scaleMode)
{
    _settings.scaleXSetting = scaleMode;

    if (scaleMode != SCALE_MANUAL)
    {
        scalePlot();
    }
}

void ScopeGui::setxAxisScale(AxisScaleOptions scaleMode, quint32 interval)
{
    _settings.scaleXSetting = scaleMode;
    _settings.xslidingInterval = interval * 1000;

    if (scaleMode != SCALE_MANUAL)
    {
        scalePlot();
    }
}

void ScopeGui::setyAxisScale(AxisScaleOptions scaleMode)
{
    _settings.scaleYSetting = scaleMode;

    if (scaleMode != SCALE_MANUAL)
    {
        scalePlot();
    }
}


void ScopeGui::setyAxisScale(AxisScaleOptions scaleMode, qint32 min, qint32 max)
{
    _settings.scaleYSetting = scaleMode;
    _settings.yMin = min;
    _settings.yMax = max;

    if (scaleMode != SCALE_MANUAL)
    {
        scalePlot();
    }
}

void ScopeGui::plotResults(QList<bool> successList, QList<double> valueList)
{
    const quint64 diff = QDateTime::currentMSecsSinceEpoch() - _scopedata->getCommunicationStartTime();

    for (qint32 i = 0; i < valueList.size(); i++)
    {
        if (successList[i])
        {
            // No error, add points
            _pPlot->graph(i)->addData(diff, valueList[i]);
            _pPlot->graph(i)->setName(QString("(%1) %2").arg(Util::formatDoubleForExport(valueList[i])).arg(_graphNames[i]));
        }
        else
        {
            _pPlot->graph(i)->addData(diff, 0);
            _pPlot->graph(i)->setName(QString("(-) %1").arg(_graphNames[i]));
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

void ScopeGui::bringToFront(quint32 index, bool bFront)
{
    if (bFront)
    {
        _pPlot->graph(index)->setLayer("topMain");
    }
    else
    {
        _pPlot->graph(index)->setLayer("grid");
    }

    _pPlot->replot();
}

QColor ScopeGui::getGraphColor(quint32 index)
{
    return _pPlot->graph(index)->pen().color();
}

void ScopeGui::exportDataCsv(QString dataFile)
{

    if (_pPlot->graphCount() != 0)
    {
        const QList<double> keyList = _pPlot->graph(0)->data()->keys();
        QList<QList<QCPData> > dataList;
        QString logData;
        QDateTime dt;
        QString line;
        QString comment = QString("//");

        logData.append(comment + "ModbusScope version" + Util::getSeparatorCharacter() + QString(APP_VERSION) + "\n");

        // Save start time
        dt = QDateTime::fromMSecsSinceEpoch(_scopedata->getCommunicationStartTime());
        logData.append(comment + "Start time" + Util::getSeparatorCharacter() + dt.toString("dd-MM-yyyy HH:mm:ss") + "\n");

        // Save end time
        dt = QDateTime::fromMSecsSinceEpoch(_scopedata->getCommunicationEndTime());
        logData.append(comment + "End time" + Util::getSeparatorCharacter() + dt.toString("dd-MM-yyyy HH:mm:ss") + "\n");

        // Export communication settings
        ModbusSettings commSettings;
        _scopedata->getSettings(&commSettings);
        logData.append(comment + "Slave IP" + Util::getSeparatorCharacter() + commSettings.getIpAddress() + ":" + QString::number(commSettings.getPort()) + "\n");
        logData.append(comment + "Slave ID" + Util::getSeparatorCharacter() + QString::number(commSettings.getSlaveId()) + "\n");
        logData.append(comment + "Time-out" + Util::getSeparatorCharacter() + QString::number(commSettings.getTimeout()) + "\n");
        logData.append(comment + "Poll interval" + Util::getSeparatorCharacter() + QString::number(commSettings.getPollTime()) + "\n");

        quint32 success;
        quint32 error;
        _scopedata->getCommunicationSettings(&success, &error);
        logData.append(comment + "Communication success" + Util::getSeparatorCharacter() + QString::number(success) + "\n");
        logData.append(comment + "Communication errors" + Util::getSeparatorCharacter() + QString::number(error) + "\n");

        logData.append("\n");

        line.clear();
        line.append("Time (ms)");
        for(qint32 i = 0; i < _pPlot->graphCount(); i++)
        {
            // Get headers
            line.append(Util::getSeparatorCharacter() + _graphNames[i]);

            // Save data lists
            dataList.append(_pPlot->graph(i)->data()->values());
        }
        line.append("\n");

        logData.append(line);

        for(qint32 i = 0; i < keyList.size(); i++)
        {
            line.clear();

            // Format time (no decimals)
            const quint64 t = static_cast<quint64>(keyList[i]);
            line.append(QString::number(t, 'f', 0));

            // Add formatted data (maximum 3 decimals, no trailing zeros)
            for(qint32 d = 0; d < dataList.size(); d++)
            {
                line.append(Util::getSeparatorCharacter() + Util::formatDoubleForExport((dataList[d])[i].value));
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

    /* Clear ticks vector */
    _tickLabels.clear();

    /* Generate correct labels */
    for (qint32 index = 0; index < ticks.size(); index++)
    {
        _tickLabels.append(createTickLabelString(ticks[index]));
    }

    /* Set labels */
    _pPlot->xAxis->setTickVectorLabels(_tickLabels);
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
        if (_pPlot->axisRect()->rangeDrag() == Qt::Horizontal)
        {
            emit updateXScalingUi(ScopeGui::SCALE_MANUAL); // change to manual scaling
        }
        else if (_pPlot->axisRect()->rangeDrag() == Qt::Vertical)
        {
            emit updateYScalingUi(ScopeGui::SCALE_MANUAL); // change to manual scaling
        }
        else
        {
            // Both
            emit updateXScalingUi(ScopeGui::SCALE_MANUAL); // change to manual scaling
            emit updateYScalingUi(ScopeGui::SCALE_MANUAL); // change to manual scaling
        }
    }
}

void ScopeGui::axisDoubleClicked(QCPAxis * axis)
{
    axis->rescale();
    _pPlot->replot();
}

void ScopeGui::writeToFile(QString filePath, QString logData)
{
    QFile file(filePath);
    if ( file.open(QIODevice::WriteOnly) ) // Remove all data from file
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

QString ScopeGui::createTickLabelString(qint32 tickKey)
{
    QString tickLabel;
    bool bNegative;
    quint32 tmp;

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
