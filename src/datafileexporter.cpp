
#include "util.h"

#include "qcustomplot.h"
#include "guimodel.h"
#include "settingsmodel.h"
#include "extendedgraphview.h"

#include "datafileexporter.h"

DataFileExporter::DataFileExporter(GuiModel *pGuiModel, SettingsModel * pSettingsModel, ExtendedGraphView * pGraphView, QObject *parent) :
    QObject(parent)
{
    _pGuiModel = pGuiModel;
    _pSettingsModel = pSettingsModel;
    _pGraphView = pGraphView;

    _bExportDuringLog = false;

    connect(_pGraphView, SIGNAL(dataAddedToPlot(double, QList<double>)), this, SLOT(exportDataLine(double, QList <double>)));
}

DataFileExporter::~DataFileExporter()
{

}

void DataFileExporter::enableExporterDuringLog()
{
    _dataExportBuffer.clear();
    lastLogTime = QDateTime::currentMSecsSinceEpoch();

    _bExportDuringLog = true;

    // Clean file
    clearFile(_pSettingsModel->writeDuringLogPath());

    exportDataHeader();
}

void DataFileExporter::disableExporterDuringLog()
{
    _bExportDuringLog = false;
    flushExportBuffer();
}

void DataFileExporter::exportDataLine(double timeData, QList <double> dataValues)
{
    if (_bExportDuringLog)
    {
        // Use buffering
        _dataExportBuffer.append(formatData(timeData, dataValues));

        if ((QDateTime::currentMSecsSinceEpoch() - lastLogTime) > _cLogBufferTimeout)
        {
            flushExportBuffer();
        }
    }
}

void DataFileExporter::exportDataFile(QString dataFile)
{
    if (_pGuiModel->graphCount() != 0)
    {
        const QList<double> keyList = _pGraphView->graphTimeData();
        QList<QList<QCPData> > dataList;
        QStringList logData;

        // Create header
        logData.append(constructDataHeader(false));

        // Create label row
        logData.append(createLabelRow());

        for(quint32 i = 0; i < _pGuiModel->graphCount(); i++)
        {
            // Save data lists
            dataList.append(_pGraphView->graphData(i));
        }

        // Add data lines
        for(qint32 i = 0; i < keyList.size(); i++)
        {
            QList<double> dataRowValues;
            for(qint32 d = 0; d < dataList.size(); d++)
            {
                dataRowValues.append((dataList[d])[i].value);
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
    writeToFile(_pSettingsModel->writeDuringLogPath(), _dataExportBuffer);

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
    writeToFile(_pSettingsModel->writeDuringLogPath(), logData);
}

QString DataFileExporter::constructDataHeader(bool bDuringLog)
{
    QString header = QString(tr(""));

    if (_pGuiModel->graphCount() != 0)
    {
        QDateTime dt;

        QString comment = QString("//");

        header.append(comment + "ModbusScope version" + Util::separatorCharacter() + QString(APP_VERSION) + "\n");

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
    for(quint32 i = 0; i < _pGuiModel->graphCount(); i++)
    {
        // Get headers
        line.append(Util::separatorCharacter() + _pGuiModel->graphLabel(i));
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

        stream << "\n";
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

void DataFileExporter::clearFile(QString filePath)
{
    QFile file(filePath);
    file.open(QIODevice::WriteOnly); // Remove all data from file
}



