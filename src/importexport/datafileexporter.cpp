
#include "util.h"

#include "qcustomplot.h"
#include "guimodel.h"
#include "settingsmodel.h"
#include "graphdatamodel.h"
#include "extendedgraphview.h"

#include "datafileexporter.h"

DataFileExporter::DataFileExporter(GuiModel *pGuiModel, SettingsModel * pSettingsModel, ExtendedGraphView * pGraphView, GraphDataModel * pGraphDataModel, QObject *parent) :
    QObject(parent)
{
    _pGuiModel = pGuiModel;
    _pSettingsModel = pSettingsModel;
    _pGraphView = pGraphView;
    _pGraphDataModel = pGraphDataModel;

    connect(_pGraphView, SIGNAL(dataAddedToPlot(double, QList<double>)), this, SLOT(exportDataLine(double, QList <double>)));
}

DataFileExporter::~DataFileExporter()
{

}

void DataFileExporter::enableExporterDuringLog()
{
    _dataExportBuffer.clear();
    lastLogTime = QDateTime::currentMSecsSinceEpoch();

    // Clean file
    clearFile(_pSettingsModel->writeDuringLogFile());

    exportDataHeader();
}

void DataFileExporter::disableExporterDuringLog()
{
    flushExportBuffer();
}

void DataFileExporter::exportDataLine(double timeData, QList <double> dataValues)
{
    /* QList correspond with activeGraphList */

    if (_pSettingsModel->writeDuringLog())
    {
        // Use buffering
        _dataExportBuffer.append(formatData(timeData, dataValues));

        if ((QDateTime::currentMSecsSinceEpoch() - lastLogTime) > _cLogBufferTimeout)
        {
            flushExportBuffer();
        }
    }
}

void DataFileExporter::rewriteDataFile(void)
{
    _dataExportBuffer.clear();
    lastLogTime = QDateTime::currentMSecsSinceEpoch();

    exportDataFile(_pSettingsModel->writeDuringLogFile());
}

void DataFileExporter::exportDataFile(QString dataFile)
{
    if (_pGraphDataModel->activeCount() != 0)
    {
        const QList<double> keyList = _pGraphView->graphTimeData();
        QList<QCPDataMap *> dataList;
        QStringList logData;

        // Create header
        logData.append(constructDataHeader(false));

        // Create label row
        logData.append(createLabelRow());

        QList<quint16> activeGraphIndexes;
        _pGraphDataModel->activeGraphIndexList(&activeGraphIndexes);

        for(qint32 idx = 0; idx < activeGraphIndexes.size(); idx++)
        {
            // Save data lists
            dataList.append(_pGraphDataModel->dataMap(activeGraphIndexes[idx]));
        }

        // Add data lines
        for(qint32 i = 0; i < keyList.size(); i++)
        {
            QList<double> dataRowValues;
            for(qint32 d = 0; d < dataList.size(); d++)
            {
                dataRowValues.append(dataList[d]->value(keyList[i]).value);
            }

            logData.append(formatData(keyList[i], dataRowValues));
        }

        // Clean file
        clearFile(dataFile);

        // Export data
        writeToFile(dataFile, logData);
    }
}

void DataFileExporter::flushExportBuffer()
{
    // Write to file
    writeToFile(_pSettingsModel->writeDuringLogFile(), _dataExportBuffer);

    _dataExportBuffer.clear();
    lastLogTime = QDateTime::currentMSecsSinceEpoch();
}

void DataFileExporter::exportDataHeader()
{
    QStringList logData;

    // create header
    logData.append(constructDataHeader(true));

    // Create label row
    logData.append(createLabelRow());

    // Write to file
    writeToFile(_pSettingsModel->writeDuringLogFile(), logData);
}

QString DataFileExporter::constructDataHeader(bool bDuringLog)
{
    QString header = QString(tr(""));

    if (_pGraphDataModel->activeCount() != 0)
    {
        QDateTime dt;

        QString comment = QString("//");

        header.append(comment + "ModbusScope version" + Util::separatorCharacter() + Util::currentVersion() + "\n");

        // Save start time
        dt = QDateTime::fromMSecsSinceEpoch(_pGuiModel->communicationStartTime());
        header.append(comment + "Start time" + Util::separatorCharacter() + dt.toString("dd-MM-yyyy HH:mm:ss") + "\n");

        // Save end time
        if (bDuringLog)
        {
            header.append(comment + "End time" + Util::separatorCharacter() + "data written during logging\n");
        }
        else
        {
            dt = QDateTime::fromMSecsSinceEpoch(_pGuiModel->communicationEndTime());
            header.append(comment + "End time" + Util::separatorCharacter() + dt.toString("dd-MM-yyyy HH:mm:ss") + "\n");
        }

        // Export communication settings
        header.append(comment + "Slave IP" + Util::separatorCharacter() + _pSettingsModel->ipAddress() + ":" + QString::number(_pSettingsModel->port()) + "\n");
        header.append(comment + "Slave ID" + Util::separatorCharacter() + QString::number(_pSettingsModel->slaveId()) + "\n");
        header.append(comment + "Time-out" + Util::separatorCharacter() + QString::number(_pSettingsModel->timeout()) + "\n");
        header.append(comment + "Poll interval" + Util::separatorCharacter() + QString::number(_pSettingsModel->pollTime()) + "\n");

        quint32 success = _pGuiModel->communicationSuccessCount();
        quint32 error = _pGuiModel->communicationErrorCount();
        header.append(comment + "Communication success" + Util::separatorCharacter() + QString::number(success) + "\n");
        header.append(comment + "Communication errors" + Util::separatorCharacter() + QString::number(error) + "\n");

        header.append("//");
    }

    return header;
}

QString DataFileExporter::createLabelRow()
{
    QString line;

    line.append("Time (ms)");
    for(qint32 i = 0; i < _pGraphDataModel->activeCount(); i++)
    {
        // Get headers
        line.append(Util::separatorCharacter() + _pGraphDataModel->label(_pGraphDataModel->convertToGraphIndex(i)));
    }

    return line;
}

QString DataFileExporter::formatData(double timeData, QList<double> dataValues)
{
    QString line;

    // Format time (no decimals)
    const quint64 t = static_cast<quint64>(timeData);
    line.append(QString::number(t, 'f', 0));

    // Add formatted data (maximum 3 decimals, no trailing zeros)
    for(qint32 d = 0; d < dataValues.size(); d++)
    {
        line.append(Util::separatorCharacter() + Util::formatDoubleForExport(dataValues[d]));
    }

    return line;
}

void DataFileExporter::writeToFile(QString filePath, QStringList logData)
{
    QFile file(filePath);
    if (file.open(QIODevice::Append))
    {
        QTextStream stream(&file);

        foreach(QString line, logData)
        {
            stream << line << "\n";
        }
    }
    else
    {
        if (
                (_pSettingsModel->writeDuringLogFile() == filePath)
                && (_pSettingsModel->writeDuringLog())
            )
        {
            // Disable logging to file on write error
            _pSettingsModel->setWriteDuringLog(false);
        }

        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("ModbusScope export error"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Save to data file (%1) failed").arg(filePath));
        msgBox.exec();
    }
}

void DataFileExporter::clearFile(QString filePath)
{
    QFile file(filePath);
    file.open(QIODevice::WriteOnly); // Remove all data from file
}



