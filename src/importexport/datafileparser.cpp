#include <QColor>
#include <QDateTime>
#include "datafileparser.h"

const QString DataFileParser::_cPattern = QString("\\s*(\\d{1,2})[\\-\\/\\s](\\d{1,2})[\\-\\/\\s](\\d{4})\\s*([0-2][0-9]):([0-5][0-9]):([0-5][0-9])[.,]?(\\d{0,3})");


DataFileParser::DataFileParser(DataParserModel *pDataParserModel)
{
    _pDataParserModel = pDataParserModel;

    _dateParseRegex.setPattern(_cPattern);
    _dateParseRegex.optimize();
}

DataFileParser::~DataFileParser()
{

}

// Return false on error
bool DataFileParser::processDataFile(QTextStream * pDataStream, FileData * pData)
{
    bool bRet = true;
    QString line;
    qint32 lineIdx = 0;

    qint64 bytesAvailable = 0;
    if (pDataStream->device() != nullptr)
    {
        bytesAvailable = pDataStream->device()->bytesAvailable();
    }
    else if (pDataStream->string() != nullptr)
    {
        bytesAvailable = pDataStream->string()->size();
    }
    else
    {
        bytesAvailable = 0;
    }

    // Get total number of bytes
    if (bytesAvailable > 0)
    {
        _totalCharSize = static_cast<quint64>(bytesAvailable);
    }
    else
    {
        _totalCharSize = 1u;
    }
    _charCount = 0;
    _lastPercentageUpdate = 0;

    pDataStream->seek(0);
    _lineNumber = 0u;

    /* Read complete file */
    if (bRet)
    {
        do
        {
            bRet = readLineFromFile(pDataStream, &line);

            if (bRet)
            {
                /* Check for properties */
                QStringList idList = line.split(_pDataParserModel->fieldSeparator());

                if (static_cast<QString>(idList.first()).toLower() == "//color")
                {
                    bool bValidColor;

                    // Remove color property name
                    idList.removeFirst();

                    foreach(QString strColor, idList)
                    {
                        bValidColor = QColor::isValidColor(strColor);
                        if (bValidColor)
                        {
                            pData->colors.append(QColor(strColor));
                        }
                        else
                        {
                            // If not valid color, then clear color list and break loop
                            pData->colors.clear();
                            break;
                        }
                    }
                }
                else if (static_cast<QString>(idList.first()).toLower() == "//note")
                {
                    Note note;
                    if (parseNoteField(idList, &note))
                    {
                        pData->notes.append(note);
                    }
                    else
                    {
                        QString error = QString(tr("Invalid note data\n"
                                                   "Line: %1\n"
                                                   ).arg(line));
                        emit parseErrorOccurred(error);
                    }
                }
            }
            else
            {
                emit parseErrorOccurred(tr("Invalid data file (while reading comments)"));
                break;
            }

            lineIdx++;
        } while(lineIdx <= _pDataParserModel->labelRow());
    }

    // Process register labels
    if (bRet)
    {
        pData->dataLabel.clear();
        pData->dataLabel.append(line.split(_pDataParserModel->fieldSeparator()));
        _expectedFields = static_cast<quint32>(pData->dataLabel.size() - static_cast<int>(_pDataParserModel->column()));

        if (_expectedFields <= 1)
        {
            emit parseErrorOccurred(QString(tr("Incorrect graph data found. "
                                 "<br><br>Found field separator: \'%1\'")).arg(_pDataParserModel->fieldSeparator()));
            bRet = false;
        }
    }

    if (bRet)
    {
        /* Clear color list when size is not ok */
        if ((pData->colors.size() + 1) != static_cast<int>(_expectedFields))
        {
             pData->colors.clear();
        }
    }

    // Trim labels
    if (bRet)
    {
        // Trim labels
        for(qint32 i = 0; i < pData->dataLabel.size(); i++)
        {
            pData->dataLabel[i] = pData->dataLabel[i].trimmed();

            if (pData->dataLabel[i].isEmpty())
            {
                pData->dataLabel[i] = QString("Unknown column %1").arg(i);
            }
        }

        // Remove earlier columns
        for (qint32 i = 0; i < static_cast<qint32>(_pDataParserModel->column()); i++)
        {
            pData->dataLabel.removeFirst();
        }

        // Save time label
        pData->axisLabel = pData->dataLabel.first();

        // Remove time label from data
        pData->dataLabel.removeFirst();
    }

    if (bRet)
    {
        // Read till data
        while(lineIdx < static_cast<qint32>(_pDataParserModel->dataRow()))
        {
            bRet = readLineFromFile(pDataStream, &line);

            if (!bRet)
            {
                emit parseErrorOccurred(tr("Invalid data file (while reading data from file)"));
                break;
            }

            lineIdx++;
        }
    }

    // read data
    if (bRet)
    {
        bRet = parseDataLines(pDataStream, pData->dataRows);

        // Time data is put on first row, rest is filtered out

		// Get time data from data
        pData->timeRow = pData->dataRows[0];

        // Remove time data from data row
        pData->dataRows.removeFirst();
    }

    if (bRet)
    {
        if (_pDataParserModel->stmStudioCorrection())
        {
            correctStmStudioData(pData->dataRows);
        }
    }

    return bRet;
}

bool DataFileParser::parseDataLines(QTextStream* pDataStream, QList<QList<double> > &dataRows)
{
    QString line;
    bool bRet = true;
    bool bResult = true;

    // Read next line
    bResult = readLineFromFile(pDataStream, &line);

    // Init data row QLists to empty list
    QList<double> t;
    for (quint32 i = 0; i < _expectedFields; i++)
    {
        dataRows.append(t);
    }

    while (bResult && bRet)
    {
        QString strippedLine = line.trimmed();

        if (
            (!strippedLine.isEmpty())
            && (!isCommentLine(strippedLine))
        )
        {
            QStringList paramList = strippedLine.split(_pDataParserModel->fieldSeparator());
            const quint32 lineDataCount = static_cast<quint32>(paramList.size() - static_cast<qint32>(_pDataParserModel->column()));

            if (lineDataCount != _expectedFields)
            {
                QString error = QString(tr("The number of label columns doesn't match number of data columns!\n\n"
                                        "Line number: %1\n"
                                        ).arg(_lineNumber));
                emit parseErrorOccurred(error);
                bRet = false;
                break;
            }

            for (qint32 i = static_cast<qint32>(_pDataParserModel->column()); i < paramList.size(); i++)
            {
                bool bOk = true;
                QString strNumber = paramList[i].simplified();
                double number;

                if (strNumber.isEmpty())
                {
                    number = 0;
                }
                else
                {
                    number = parseDouble(strNumber, &bOk);
                }

                if (
                    (bOk == false)
                    && (static_cast<quint32>(i) == _pDataParserModel->column())
                )
                {
                    // Parse time data
                    number = static_cast<double>(parseDateTime(strNumber, &bOk));

                    if (!bOk)
                    {
                        QString error = QString(tr("Invalid absolute date (while processing data)\n"
                                                   "Line number: %1\n"
                                                   "Line: \"%2\"\n"
                                                   "\n\nExpected date format: \'%3\'"
                                                   ).arg(_lineNumber).arg(strippedLine, "dd-MM-yyyy hh:mm:ss.zzz"));
                        emit parseErrorOccurred(error);
                        bRet = false;
                        break;
                    }
                }

                if (bOk)
                {
                    /* Only multiply for first column (time data) */
                    if (
                        (static_cast<quint32>(i) == _pDataParserModel->column())
                        && (!_pDataParserModel->timeInMilliSeconds())
                        )
                    {
                        number *= 1000;
                    }

                    dataRows[i - static_cast<qint32>(_pDataParserModel->column())].append(number);
                }
                else
                {
                    QString error = QString(tr("Invalid data (while processing data)\n"
                                               "Line number: %1\n"
                                               "Line: \"%2\"\n"
                                               "\n\nExpected decimal separator character: \'%3\'"
                                               ).arg(_lineNumber).arg(strippedLine).arg(_pDataParserModel->decimalSeparator()));
                    emit parseErrorOccurred(error);
                    bRet = false;
                    break;
                }
            }
        }

        // Check end of file
        if (pDataStream->atEnd())
        {
            break;
        }

        // Read next line
        bResult = readLineFromFile(pDataStream, &line);

    }

    return bRet;
}

// Return false on error
bool DataFileParser::readLineFromFile(QTextStream* pDataStream, QString *pLine)
{
    bool bRet = false;

    // Read line of data (skip empty line)
    do
    {
        bRet = pDataStream->readLineInto(pLine, 0);
        _lineNumber++;

    } while (bRet && pLine->trimmed().isEmpty());

    /*
     * Moved outside of loop for some performance optimizations
     * But this means that a small deviation will be created
    */
    if (bRet)
    {
        checkProgressUpdate(static_cast<quint32>(pLine->size()));
    }

    return bRet;
}

qint64 DataFileParser::parseDateTime(QString rawData, bool *bOk)
{
    QRegularExpressionMatch match = _dateParseRegex.match(rawData);

    QString day;
    QString month;
    QString year;
    QString hours;
    QString minutes;
    QString seconds;
    QString milliseconds;

    if (match.hasMatch())
    {
        day = match.captured(1);
        month = match.captured(2);
        year = match.captured(3);
        hours = match.captured(4);
        minutes = match.captured(5);
        seconds = match.captured(6);
        milliseconds = match.captured(7);
    }
    if (milliseconds.isEmpty())
    {
        milliseconds = "0";
    }

    QString dateStr = QString("%1-%2-%3 %4:%5:%6.%7").arg(day,2, '0')
                        .arg(month,2, '0')
                        .arg(year)
                        .arg(hours,2, '0')
                        .arg(minutes,2, '0')
                        .arg(seconds,2, '0')
                        .arg(milliseconds,3, '0');

    const QDateTime date = QDateTime::fromString(dateStr, "dd-MM-yyyy hh:mm:ss.zzz");

    *bOk = date.isValid();

    return date.toMSecsSinceEpoch();

}

bool DataFileParser::parseNoteField(QStringList noteFieldList, Note * pNote)
{

    /* We expect
     *
     * 0: "//Note"
     * 1: Key (double)
     * 2: Value (double)
     * 3: Note text
     */
    bool bSucces = true;

    bool bOk = true; // tmp variable

    if (noteFieldList.size() == 4)
    {
        QPointF notePosition;
        notePosition.setX(parseDouble(noteFieldList[1], &bOk));

        if (bOk)
        {
            notePosition.setY(parseDouble(noteFieldList[2], &bOk));
        }
        else
        {
            bSucces = false;
        }

        if (bOk)
        {
            pNote->setNotePosition(notePosition);
        }
        else
        {
            bSucces = false;
        }

        QRegularExpression trimStringRegex("\"?(.[^\"]*)\"?");
        pNote->setText(trimStringRegex.match(noteFieldList[3]).captured(1));
    }

    return bSucces;
}

double DataFileParser::parseDouble(QString strNumber, bool* bOk)
{
    double number = 0;

    /* Assume success */
    *bOk = true;

    // Remove group separator
    QString tmpData = strNumber.simplified().replace(_pDataParserModel->groupSeparator(), "");

    // Replace decimal point if needed
    if (QLocale().decimalPoint() != _pDataParserModel->decimalSeparator())
    {
        tmpData = tmpData.replace(_pDataParserModel->decimalSeparator(), QLocale().decimalPoint());
    }

    if (tmpData.simplified().isEmpty())
    {
        number = 0;
        *bOk = false;
    }
    else
    {
        number = QLocale().toDouble(tmpData, bOk);
    }

    return number;
}

bool DataFileParser::isCommentLine(QString line)
{
    bool bRet = false;

    const QString commentSequence = _pDataParserModel->commentSequence();
    if (!commentSequence.isEmpty())
    {
        if (line.trimmed().left(commentSequence.length()) == commentSequence)
        {
            bRet = true;
        }
    }

    return bRet;
}


void DataFileParser::checkProgressUpdate(quint32 charRead)
{
    quint32 percentage = 0;

    _charCount += charRead;

    percentage = static_cast<quint32>((_charCount * 100) / _totalCharSize);

    // Clip percentage
    if (percentage > 100)
    {
        percentage = 99;
    }

    if (percentage > _lastPercentageUpdate)
    {
        _lastPercentageUpdate = percentage;

        emit updateProgress(static_cast<int>(_lastPercentageUpdate));
    }
}

void DataFileParser::correctStmStudioData(QList<QList<double> > &dataLists)
{
    for (qint32 idx = 0; idx < dataLists.size(); idx++)
    {
        /* We need at least 3 points */
        if (dataLists[idx].size() > 3)
        {
            /* Skip first and last point */
            for (int32_t pointIdx = 1; pointIdx < dataLists[idx].size() - 1; pointIdx++)
            {
                const int32_t leftPoint = dataLists[idx][pointIdx - 1];
                const int32_t refPoint = dataLists[idx][pointIdx];
                const int32_t rightPoint = dataLists[idx][pointIdx + 1];

                /* Only correct 16 bit variables */
                if (
                    (refPoint < 65535)
                    && (refPoint > 0)
                    )
                {
                    const uint32_t leftDiff = qAbs(leftPoint - refPoint);
                    const uint32_t rightDiff = qAbs(refPoint - rightPoint);
                    const uint32_t outerDiff = qAbs(leftPoint - rightPoint);

                    /* difference between samples should be large enough */
                    if (
                        (leftDiff > (2 * outerDiff))
                        && (rightDiff > (2 * outerDiff))
                    )
                    {
                        if (isNibbleCorrupt(static_cast<quint16>(refPoint), leftPoint))
                        {
                            dataLists[idx][pointIdx] = leftPoint;
                        }
                        else if (isNibbleCorrupt(static_cast<quint16>(refPoint), rightPoint))
                        {
                            dataLists[idx][pointIdx] = rightPoint;
                        }
                        else
                        {
                            /* No change needed */
                        }

                    }

                }
            }

        }
    }
}

bool DataFileParser::isNibbleCorrupt(quint16 ref, quint16 compare)
{
    if (
            /* Zeroed nibbles */
            (ref == (compare & 0xFF00))
            || (ref == (compare & 0x00FF))
            /* Nibbles set to ones */
            || (ref == (compare | 0xFF00))
            || (ref == (compare | 0x00FF))
            || (ref == 0)
        )
    {
        return true;
    }
    return false;
}
