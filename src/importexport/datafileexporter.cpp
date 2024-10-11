
#include "util.h"
#include "formatdatetime.h"

#include "qcustomplot.h"
#include "settingsmodel.h"
#include "graphdatamodel.h"

#include "datafileexporter.h"
#include "notemodel.h"

DataFileExporter::DataFileExporter(SettingsModel * pSettingsModel, GraphDataModel * pGraphDataModel, NoteModel *pNoteModel, QObject *parent) :
    QObject(parent)
{
    _pSettingsModel = pSettingsModel;
    _pGraphDataModel = pGraphDataModel;
    _pNoteModel = pNoteModel;

    lastLogTime = QDateTime::currentMSecsSinceEpoch();
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
            const qint32 dataCount = _pGraphDataModel->dataMap(activeGraphIndexes[0])->size();
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

bool DataFileExporter::updateNoteLines(QString dataFile)
{
    bool bSuccess = true;

    QFileInfo fileInfo(dataFile);
    QString tmpFileName = QString("%1/tmp_%2").arg(fileInfo.path(), fileInfo.fileName());

    QFile srcFile(dataFile);
    QFile tmpFile(tmpFileName);

    if (srcFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (tmpFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            // Clear tmp file
            tmpFile.resize(0);

            QTextStream srcStream(&srcFile);
            QTextStream tmpStream(&tmpFile);
            QString line;
            bool bPreviousLineWasEmptyComment = false;

            while(!srcStream.atEnd())
            {
                line = srcStream.readLine().trimmed();

                if (line.left(2) == "//")
                {
                    bool bEmptyCommentLine = line.length() == 2;

                    if (line.left(6).toLower() == "//note")
                    {
                        // Skip line
                    }
                    else if (bPreviousLineWasEmptyComment && bEmptyCommentLine)
                    {
                        // We want to avoid 2 subsequent comment lines, so skip
                    }
                    else
                    {
                        // copy line
                        tmpStream << line << "\n";
                        bPreviousLineWasEmptyComment = bEmptyCommentLine;
                    }
                }
                else
                {
                    // No comment line
                    // Keep first data line in variable to insert after notes
                    break;
                }
            }

            // Add notes
            QStringList noteRows;
            createNoteRows(noteRows);
            if (!noteRows.isEmpty())
            {
                for(const QString &noteRow: std::as_const(noteRows))
                {
                    tmpStream << noteRow << "\n";
                }

                tmpStream << "//\n";
            }

            // Copy last line
            tmpStream << line << "\n";

            // Copy rest of the file
            while(!srcStream.atEnd())
            {
                tmpStream << srcStream.readLine() << "\n";
            }
        }
        else
        {
            bSuccess = false;
        }

    }
    else
    {
        bSuccess = false;
    }

    srcFile.close();
    tmpFile.close();

    if (bSuccess)
    {
        bSuccess = srcFile.remove();

        if (bSuccess)
        {
            bSuccess = tmpFile.rename(dataFile);
        }
    }

    if (bSuccess)
    {
        _pNoteModel->setNotesDataUpdated(false);
        return true;
    }
    else
    {
        Util::showError(tr("Update notes in data file (%1) failed").arg(dataFile));
        return false;
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
        dt = QDateTime::fromMSecsSinceEpoch(_pGraphDataModel->communicationStartTime());
        header.append(comment + "Start time" + Util::separatorCharacter() + dt.toString("dd-MM-yyyy HH:mm:ss"));

        // Save end time
        if (bDuringLog)
        {
            header.append(comment + "End time" + Util::separatorCharacter() + "data written during logging");
        }
        else
        {
            dt = QDateTime::fromMSecsSinceEpoch(_pGraphDataModel->communicationEndTime());
            header.append(comment + "End time" + Util::separatorCharacter() + dt.toString("dd-MM-yyyy HH:mm:ss"));
        }

        // Export communication settings
        for (quint8 i = 0u; i < Connection::ID_CNT; i++)
        {
            if (_pSettingsModel->connectionState(i))
            {
                header.append(comment + constructConnSettings(i));
                header.append(comment + "Slave ID (Connection ID " + QString::number(i + 1) + ")" + Util::separatorCharacter() + QString::number(_pSettingsModel->slaveId(i)));
                header.append(comment + "Time-out (Connection ID " + QString::number(i + 1) + ")" + Util::separatorCharacter() + QString::number(_pSettingsModel->timeout(i)));
                header.append(comment + "Consecutive max (Connection ID " + QString::number(i + 1) + ")" + Util::separatorCharacter() + QString::number(_pSettingsModel->consecutiveMax(i)));
            }
        }

        header.append(comment + "Poll interval" + Util::separatorCharacter() + QString::number(_pSettingsModel->pollTime()));

        quint32 success = _pGraphDataModel->communicationSuccessCount();
        quint32 error = _pGraphDataModel->communicationErrorCount();
        header.append(comment + "Communication success" + Util::separatorCharacter() + QString::number(success));
        header.append(comment + "Communication errors" + Util::separatorCharacter() + QString::number(error));

        header.append("//");

        header.append("//" + createPropertyRow(E_PROPERTY));
        header.append("//" + createPropertyRow(E_COLOR));
        header.append("//" + createPropertyRow(E_EXPRESSION));
        header.append("//" + createPropertyRow(E_VALUE_AXIS));

        header.append("//");

        QStringList noteRows;
        createNoteRows(noteRows);
        if (!noteRows.isEmpty())
        {
            header.append(noteRows);
            header.append("//");
        }
    }

    return header;
}

QString DataFileExporter::constructConnSettings(quint8 connectionId)
{
    QString strSettings;

    if (_pSettingsModel->connectionType(connectionId) == Connection::TYPE_TCP)
    {
        strSettings = _pSettingsModel->ipAddress(connectionId) + ":" + QString::number(_pSettingsModel->port(connectionId));
    }
    else
    {
        QString strParity;
        QString strDataBits;
        QString strStopBits;
        _pSettingsModel->serialConnectionStrings(connectionId, strParity, strDataBits, strStopBits);

        strSettings = QString("%0%1%2%3%4%5%6%7%8")
                             .arg(_pSettingsModel->portName(connectionId))
                             .arg(Util::separatorCharacter())
                             .arg(_pSettingsModel->baudrate(connectionId))
                             .arg(Util::separatorCharacter())
                             .arg(strParity)
                             .arg(Util::separatorCharacter())
                             .arg(strDataBits)
                             .arg(Util::separatorCharacter())
                             .arg(strStopBits);

    }

    return QString("Settings (Connection ID %0)%1%2")
                          .arg(connectionId + 1)
                          .arg(Util::separatorCharacter())
                          .arg(strSettings);
}

void DataFileExporter::createNoteRows(QStringList& noteRows)
{
    for (qint32 idx = 0; idx < _pNoteModel->size(); idx++)
    {
        QString noteline;
        QString dataString;

        noteline.append("//Note");

        const QPointF& position = _pNoteModel->notePosition(idx);

        dataString = Util::formatDoubleForExport(position.x());
        noteline.append(Util::separatorCharacter() + dataString);

        dataString = Util::formatDoubleForExport(position.y());
        noteline.append(Util::separatorCharacter() + dataString);

        noteline.append(Util::separatorCharacter() + '"' + _pNoteModel->textData(idx) + '"');

        noteRows.append(noteline);
    }
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

    case E_EXPRESSION:
        line.append("Expression");
        break;

    case E_VALUE_AXIS:
        line.append("Axis");

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

        case E_EXPRESSION:
            propertyString = QString("\"%1\"").arg(_pGraphDataModel->simplifiedExpression(graphIdx));
            break;

        case E_VALUE_AXIS:
            {
                qint32 axis = _pGraphDataModel->valueAxis(graphIdx) == GraphData::VALUE_AXIS_PRIMARY ? 0 : 1;
                propertyString = QString("%1").arg(axis);
            }
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
        QDateTime dateTime;
        dateTime.setMSecsSinceEpoch(timeData);
        line.append(FormatDateTime::formatDateTime(dateTime));
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

        Util::showError(tr("Save to data file (%1) failed").arg(filePath));
    }

    return bRet;
}

void DataFileExporter::clearFile(QString filePath)
{
    QFile file(filePath);
    (void)file.open(QIODevice::WriteOnly | QIODevice::Text); // Remove all data from file
}



