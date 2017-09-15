
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
        const qint32 dataCount = _pGraphView->graphDataSize();
        QStringList logData;

        // Create header
        logData.append(constructDataHeader(false));

        // Create label row
        logData.append(createPropertyRow(E_LABEL));

        // Clean file
        clearFile(dataFile);

        // Export header data
        bool bRet = writeToFile(dataFile, logData);

        logData.clear();

        if (bRet)
        {
            QList<quint16> activeGraphIndexes;
            _pGraphDataModel->activeGraphIndexList(&activeGraphIndexes);
            QList<QCPGraphDataContainer::const_iterator> dataListIterators;

            for(qint32 idx = 0; idx < activeGraphIndexes.size(); idx++)
            {
                // Save iterators to data lists
                dataListIterators.append(_pGraphDataModel->dataMap(activeGraphIndexes[idx])->constBegin());
            }

            // Add data lines
            for(qint32 i = 0; i < dataCount; i++)
            {
                QList<double> dataRowValues;
                double key = dataListIterators[0]->key;
                for(qint32 d = 0; d < dataListIterators.size(); d++)
                {
                    dataRowValues.append(dataListIterators[d]->value);

                    dataListIterators[d]++;
                }

                logData.append(formatData(key, dataRowValues));


                if ( i % _cLogChunkLineCount == 0)
                {
                    bRet = writeToFile(dataFile, logData);

                    logData.clear();

                    if (!bRet)
                    {
                        break;
                    }
                }
            }

            if (bRet && (logData.size() > 0))
            {
                writeToFile(dataFile, logData);
            }
        }
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
    logData.append(createPropertyRow(E_LABEL));

    // Write to file
    writeToFile(_pSettingsModel->writeDuringLogFile(), logData);
}

QStringList DataFileExporter::constructDataHeader(bool bDuringLog)
{
    QStringList header;

    if (_pGraphDataModel->activeCount() != 0)
    {
        QDateTime dt;

        QString comment = QString("//");

        header.append(comment + "ModbusScope version" + Util::separatorCharacter() + Util::currentVersion());

        // Save start time
        dt = QDateTime::fromMSecsSinceEpoch(_pGuiModel->communicationStartTime());
        header.append(comment + "Start time" + Util::separatorCharacter() + dt.toString("dd-MM-yyyy HH:mm:ss"));

        // Save end time
        if (bDuringLog)
        {
            header.append(comment + "End time" + Util::separatorCharacter() + "data written during logging");
        }
        else
        {
            dt = QDateTime::fromMSecsSinceEpoch(_pGuiModel->communicationEndTime());
            header.append(comment + "End time" + Util::separatorCharacter() + dt.toString("dd-MM-yyyy HH:mm:ss"));
        }

        // Export communication settings
        header.append(comment + "Slave IP" + Util::separatorCharacter() + _pSettingsModel->ipAddress() + ":" + QString::number(_pSettingsModel->port()));
        header.append(comment + "Slave ID" + Util::separatorCharacter() + QString::number(_pSettingsModel->slaveId()));
        header.append(comment + "Time-out" + Util::separatorCharacter() + QString::number(_pSettingsModel->timeout()));
        header.append(comment + "Poll interval" + Util::separatorCharacter() + QString::number(_pSettingsModel->pollTime()));

        quint32 success = _pGuiModel->communicationSuccessCount();
        quint32 error = _pGuiModel->communicationErrorCount();
        header.append(comment + "Communication success" + Util::separatorCharacter() + QString::number(success));
        header.append(comment + "Communication errors" + Util::separatorCharacter() + QString::number(error));

        header.append("//");

        header.append("//" + createPropertyRow(E_PROPERTY));
        header.append("//" + createPropertyRow(E_REGISTER_ADDRESS));
        header.append("//" + createPropertyRow(E_COLOR));
        header.append("//" + createPropertyRow(E_MULTIPLY_FACTOR));
        header.append("//" + createPropertyRow(E_DIVIDE_FACTOR));
        header.append("//" + createPropertyRow(E_BITMASK));
        header.append("//" + createPropertyRow(E_SHIFT));

        header.append("//");
    }

    return header;
}

QString DataFileExporter::createPropertyRow(registerProperty prop)
{
    QString line;

    switch(prop)
    {
    case E_LABEL:
        line.append("Time (ms)");
        break;

    case E_PROPERTY:
        line.append("Property");
        break;

    case E_COLOR:
        line.append("Color");
        break;

    case E_MULTIPLY_FACTOR:
        line.append("Multiply");
        break;

    case E_DIVIDE_FACTOR:
        line.append("Divide");
        break;

    case E_REGISTER_ADDRESS:
        line.append("Register Address");
        break;

    case E_BITMASK:
        line.append("Bitmask");
        break;

    case E_SHIFT:
        line.append("Shift");
        break;

    default:
        break;
    }

    for(qint32 i = 0; i < _pGraphDataModel->activeCount(); i++)
    {
        const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(i);

        QString propertyString;
        switch(prop)
        {
        case E_LABEL:
        case E_PROPERTY:
            propertyString = _pGraphDataModel->label(graphIdx);
            break;
        case E_COLOR:
            propertyString = _pGraphDataModel->color(graphIdx).name();
            break;

        case E_MULTIPLY_FACTOR:
            propertyString = Util::formatDoubleForExport(_pGraphDataModel->multiplyFactor(graphIdx));
            break;

        case E_DIVIDE_FACTOR:
            propertyString =  Util::formatDoubleForExport(_pGraphDataModel->divideFactor(graphIdx));
            break;

        case E_REGISTER_ADDRESS:
            propertyString = QString("%1").arg(_pGraphDataModel->registerAddress(graphIdx));
            break;

        case E_BITMASK:
            propertyString = QString("0x%1").arg(_pGraphDataModel->bitmask(graphIdx), 0, 16);
            break;

        case E_SHIFT:
            propertyString = QString("%1").arg(_pGraphDataModel->shift(graphIdx));
            break;

        default:
            break;

        }

        // Get headers
        line.append(Util::separatorCharacter() + propertyString);
    }

    return line;
}

QString DataFileExporter::formatData(double timeData, QList<double> dataValues)
{
    QString line;

    if (_pSettingsModel->absoluteTimes())
    {
        QString timeString;
        QDateTime dateTime;
        dateTime.setMSecsSinceEpoch(timeData);
        timeString = dateTime.toString("dd/MM/yyyy " + Util::timeStringFormat());
        line.append(timeString);
    }
    else
    {
        // Format time (no decimals)
        const quint64 t = static_cast<quint64>(timeData);
        line.append(QString::number(t, 'f', 0));
    }

    // Add formatted data (maximum 3 decimals, no trailing zeros)
    for(qint32 d = 0; d < dataValues.size(); d++)
    {
        line.append(Util::separatorCharacter() + Util::formatDoubleForExport(dataValues[d]));
    }

    return line;
}

bool DataFileExporter::writeToFile(QString filePath, QStringList logData)
{
    bool bRet = false;
    QFile file(filePath);
    if (file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream stream(&file);

        foreach(QString line, logData)
        {
            stream << line << "\n";
        }

        bRet = true;
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

    return bRet;
}

void DataFileExporter::clearFile(QString filePath)
{
    QFile file(filePath);
    file.open(QIODevice::WriteOnly | QIODevice::Text); // Remove all data from file
}



