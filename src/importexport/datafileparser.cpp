
#include <QtWidgets>

#include "util.h"
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
    if (_pDataStream)
    {
        delete _pDataStream;
    }
}

// Return false on error
bool DataFileParser::processDataFile(QTextStream * pDataStream, FileData * pData)
{
    bool bRet = true;
    QString line;
    qint32 lineIdx = 0;

    _pDataStream = pDataStream;
    _pDataStream->seek(0);

    /* Read complete file */
    if (bRet)
    {
        do
        {
            bRet = readLineFromFile(&line);

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
                        Util::showError(error);
                    }
                }
            }
            else
            {
                Util::showError(tr("Invalid data file (while reading comments)"));
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
            Util::showError(QString(tr("Incorrect graph data found. "
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
		// axis (time) label
        pData->axisLabel = pData->dataLabel[0].trimmed();

        for(qint32 i = static_cast<qint32>(_pDataParserModel->column()); i < pData->dataLabel.size(); i++)
        {
            pData->dataLabel[i - static_cast<qint32>(_pDataParserModel->column())] = pData->dataLabel[i].trimmed();
        }
    }

    if (bRet)
    {
        // Read till data
        do
        {
            bRet = readLineFromFile(&line);

            if (!bRet)
            {
                Util::showError(tr("Invalid data file (while reading data from file)"));
            }

            lineIdx++;
        } while(lineIdx <= static_cast<qint32>(_pDataParserModel->dataRow()));
    }

    // read data
    if (bRet)
    {
        bRet = parseDataLines(pData->dataRows);

		// Get time data from data
        pData->timeRow = pData->dataRows[0];

		// Remove time data
        pData->dataLabel.removeAt(0);
    }

    return bRet;
}

bool DataFileParser::parseDataLines(QList<QList<double> > &dataRows)
{
    QString line;
    bool bRet = true;
    bool bResult = true;

    // Read next line
    bResult = readLineFromFile(&line);

    // Init data row QLists to empty list
    QList<double> t;
    for (quint32 i = 0; i < _expectedFields; i++)
    {
        dataRows.append(t);
    }

    while (bResult && bRet)
    {
        if (!line.trimmed().isEmpty())
        {
            QStringList paramList = line.split(_pDataParserModel->fieldSeparator());
            const quint32 lineDataCount = static_cast<quint32>(paramList.size() - static_cast<qint32>(_pDataParserModel->column()));

            if (lineDataCount != _expectedFields)
            {
                QString txt = QString(tr("The number of label columns doesn't match number of data columns!\nLabel count: %1\nData count: %2")).arg(_expectedFields).arg(lineDataCount);
                Util::showError(txt);
                bRet = false;
                break;
            }

            for (qint32 i = static_cast<qint32>(_pDataParserModel->column()); i < paramList.size(); i++)
            {
                bool bOk = true;
                QString strNumber = paramList[i].simplified();

                double number = parseDouble(strNumber, &bOk);

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
                                                   "Line: %1\n"
                                                   "\n\nExpected date format: \'%2\'"
                                                   ).arg(line).arg("dd-MM-yyyy hh:mm:ss.zzz"));
                        Util::showError(error);
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
                                               "Line: %1\n"
                                               "\n\nExpected decimal separator character: \'%2\'"
                                               ).arg(line).arg(_pDataParserModel->decimalSeparator()));
                    Util::showError(error);
                    bRet = false;
                    break;
                }
            }
        }

        // Check end of file
        if (_pDataStream->atEnd())
        {
            break;
        }

        // Read next line
        bResult = readLineFromFile(&line);

    }

    return bRet;
}

// Return false on error
bool DataFileParser::readLineFromFile(QString *pLine)
{
    // Read line of data
    return _pDataStream->readLineInto(pLine, 0);
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
        const double key = parseDouble(noteFieldList[1], &bOk);
        if (bOk)
        {
            pNote->setKeyData(key);
        }
        else
        {
            bSucces = false;
        }

        const double value = parseDouble(noteFieldList[2], &bOk);
        if (bOk)
        {
            pNote->setValueData(value);
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
    if (QLocale::system().decimalPoint() != _pDataParserModel->decimalSeparator())
    {
        tmpData = tmpData.replace(_pDataParserModel->decimalSeparator(), QLocale::system().decimalPoint());
    }

    if (tmpData.simplified().isEmpty())
    {
        number = 0;
        *bOk = false;
    }
    else
    {
        number = QLocale::system().toDouble(tmpData, bOk);
    }

    return number;
}
