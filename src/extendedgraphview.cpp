
#include "util.h"
#include "guimodel.h"
#include "extendedgraphview.h"


ExtendedGraphView::ExtendedGraphView(GuiModel * pGuiModel, QCustomPlot * pPlot, QObject *parent):
    BasicGraphView(pGuiModel, pPlot)
{
    Q_UNUSED(parent);

    connect(_pPlot->xAxis, SIGNAL(rangeChanged(QCPRange, QCPRange)), this, SLOT(xAxisRangeChanged(QCPRange, QCPRange)));

    // TODO: _settings.yMin = 0;
    // TODO: _settings.yMax = 10;
    // TODO: _pPlot->yAxis->setRange(_settings.yMin, _settings.yMax);

    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(_pPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMove(QMouseEvent*)));
    // TODO: connect(this, SIGNAL(updateXScalingUi(int)), _window,SLOT(changeXAxisScaling(int)));
    // TODO: connect(this, SIGNAL(updateYScalingUi(int)), _window,SLOT(changeYAxisScaling(int)));
}

ExtendedGraphView::~ExtendedGraphView()
{

}

void ExtendedGraphView::addGraphs(QList<QList<double> > data)
{
    BasicGraphView::addGraphs();

    updateData(&data);
}

void ExtendedGraphView::plotResults(QList<bool> successList, QList<double> valueList)
{
    const quint64 diff = QDateTime::currentMSecsSinceEpoch() - _scopedata->getCommunicationStartTime();

    for (qint32 i = 0; i < valueList.size(); i++)
    {
        if (successList[i])
        {
            // No error, add points
            _pPlot->graph(i)->addData(diff, valueList[i]);

            _pPlot->graph(i)->setName(QString("(%1) %2").arg(Util::formatDoubleForExport(valueList[i])).arg(_pGuiModel->graphLabel(i)));
        }
        else
        {
            _pPlot->graph(i)->addData(diff, 0);
            _pPlot->graph(i)->setName(QString("(-) %1").arg(_pGuiModel->graphLabel(i)));
        }
    }

   scalePlot();

}

void ExtendedGraphView::clearResults()
{
    for (qint32 i = 0; i < _pPlot->graphCount(); i++)
    {
        _pPlot->graph(i)->clearData();
        _pPlot->graph(i)->setName(QString("(-) %1").arg(_pGuiModel->graphLabel(i)));
    }

   scalePlot();
}

void ExtendedGraphView::exportDataCsv(QString dataFile)
{
#if 0
    TODO: fix
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
        logData.append(comment + "Slave IP" + Util::getSeparatorCharacter() + commSettings.getIpAddress() + ":" + QString::number(commSettings.getPort()) + "\n");
        logData.append(comment + "Slave ID" + Util::getSeparatorCharacter() + QString::number(commSettings.getSlaveId()) + "\n");
        logData.append(comment + "Time-out" + Util::getSeparatorCharacter() + QString::number(commSettings.getTimeout()) + "\n");
        logData.append(comment + "Poll interval" + Util::getSeparatorCharacter() + QString::number(commSettings.getPollTime()) + "\n");

        quint32 success;
        quint32 error;
        _scopedata->getCommunicationSettings(&success, &error);
        logData.append(comment + "Communication success" + Util::getSeparatorCharacter() + QString::number(success) + "\n");
        logData.append(comment + "Communication errors" + Util::getSeparatorCharacter() + QString::number(error) + "\n");

        logData.append("//\n");

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
#endif
}

void ExtendedGraphView::scalePlot()
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
            _pPlot->yAxis->rescale(true);
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

void ExtendedGraphView::mouseMove(QMouseEvent *event)
{
    // Check for graph drag
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
    else if  (_bEnableTooltip) // Check to show tooltip
    {
        BasicGraphView::paintValueToolTip(event);
    }
    else
    {
        if (QToolTip::isVisible())
        {
            QToolTip::hideText();
        }
    }
}

void ExtendedGraphView::updateData(QList<QList<double> > * pDataLists)
{
    bool bFullScale = false;
    const QVector<double> timeData = pDataLists->at(0).toVector();

    if (_pPlot->graph(0)->data()->keys().size() > 0)
    {
        if (
        (_pPlot->xAxis->range().lower <= _pPlot->graph(0)->data()->keys().first())
        && (_pPlot->xAxis->range().upper >= _pPlot->graph(0)->data()->keys().last())
        )
        {
            bFullScale = true;
        }
    }
    else
    {
        /* First load of file: always rescale */
        bFullScale = true;
    }

    for (qint32 i = 1; i < pDataLists->size(); i++)
    {
        //Add data to graphs
        QVector<double> graphData = pDataLists->at(i).toVector();
        _pPlot->graph(i - 1)->setData(timeData, graphData);
    }

    if (bFullScale)
    {
        _pPlot->rescaleAxes(true);
    }
    _pPlot->replot();
}


void ExtendedGraphView::writeToFile(QString filePath, QString logData)
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
