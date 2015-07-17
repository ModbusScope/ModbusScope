
#include "util.h"
#include "qcustomplot.h"

#include "datafileexporter.h"

DataFileExporter::DataFileExporter(GuiModel *pGuiModel, LogModel * pLogModel, ConnectionModel * pConnectionModel, ExtendedGraphView * pGraphView, QObject *parent) :
    QObject(parent)
{
    _pGuiModel = pGuiModel;
    _pLogModel = pLogModel;
    _pConnectionModel = pConnectionModel;
    _pGraphView = pGraphView;
}

DataFileExporter::~DataFileExporter()
{

}

void DataFileExporter::exportDataCsv(QString dataFile)
{
    if (_pGuiModel->graphCount() != 0)
    {
        const QList<double> keyList = _pGraphView->graphTimeData();
        QList<QList<QCPData> > dataList;
        QString logData;
        QDateTime dt;
        QString line;
        QString comment = QString("//");

        logData.append(comment + "ModbusScope version" + Util::separatorCharacter() + QString(APP_VERSION) + "\n");

        // Save start time
        dt = QDateTime::fromMSecsSinceEpoch(_pGuiModel->communicationStartTime());
        logData.append(comment + "Start time" + Util::separatorCharacter() + dt.toString("dd-MM-yyyy HH:mm:ss") + "\n");

        // Save end time
        dt = QDateTime::fromMSecsSinceEpoch(_pGuiModel->communicationEndTime());
        logData.append(comment + "End time" + Util::separatorCharacter() + dt.toString("dd-MM-yyyy HH:mm:ss") + "\n");

        // Export communication settings
        logData.append(comment + "Slave IP" + Util::separatorCharacter() + _pConnectionModel->ipAddress() + ":" + QString::number(_pConnectionModel->port()) + "\n");
        logData.append(comment + "Slave ID" + Util::separatorCharacter() + QString::number(_pConnectionModel->slaveId()) + "\n");
        logData.append(comment + "Time-out" + Util::separatorCharacter() + QString::number(_pConnectionModel->timeout()) + "\n");
        logData.append(comment + "Poll interval" + Util::separatorCharacter() + QString::number(_pLogModel->pollTime()) + "\n");

        quint32 success = _pGuiModel->communicationSuccessCount();
        quint32 error = _pGuiModel->communicationErrorCount();
        logData.append(comment + "Communication success" + Util::separatorCharacter() + QString::number(success) + "\n");
        logData.append(comment + "Communication errors" + Util::separatorCharacter() + QString::number(error) + "\n");

        logData.append("//\n");

        line.clear();
        line.append("Time (ms)");
        for(quint32 i = 0; i < _pGuiModel->graphCount(); i++)
        {
            // Get headers
            line.append(Util::separatorCharacter() + _pGuiModel->graphLabel(i));

            // Save data lists
            dataList.append(_pGraphView->graphData(i));
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
                line.append(Util::separatorCharacter() + Util::formatDoubleForExport((dataList[d])[i].value));
            }
            line.append("\n");

            logData.append(line);
        }

        writeToFile(dataFile, logData);
    }
}

void DataFileExporter::writeToFile(QString filePath, QString logData)
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


