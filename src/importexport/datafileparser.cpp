
#include <QtWidgets>
#include <QMessageBox>
#include "datafileparser.h"

const QString DataFileParser::_cPattern = QString("\\s*(\\d{1,2})[\\-\\/\\s](\\d{1,2})[\\-\\/\\s](\\d{4})\\s*([0-2][0-9]):([0-5][0-9]):([0-5][0-9])[.,]?(\\d{0,3})");


DataFileParser::DataFileParser()
{
    _dateParseRegex.setPattern(_cPattern);
    _dateParseRegex.optimize();
}

DataFileParser::~DataFileParser()
{
    if (_pDataStream)
    {
        delete _pDataStream;
    }

    if (_pAutoSettingsParser)
    {
       delete _pAutoSettingsParser;
    }
}

// Return false on error
bool DataFileParser::processDataFile(QString dataFile, FileData * pData)
{
    bool bRet = true;
    QString line;
    qint32 lineIdx = 0;
    QStringList dataFileSample;

    QFile * _pDataFile = new QFile(dataFile);

    /* Read sample of file */
    bRet = _pDataFile->open(QIODevice::ReadOnly | QIODevice::Text);
    if (bRet)
    {
        _pDataStream = new QTextStream(_pDataFile);

        loadDataFileSample(&dataFileSample);
    }
    else
    {
        showError(tr("Couldn't open data file: %1").arg(dataFile));
    }

    /* Try to determine settings */
    if (bRet)
    {
        _pAutoSettingsParser = new SettingsAuto();
        bRet = _pAutoSettingsParser->updateSettings(dataFileSample);

        if (!bRet)
        {
            showError(tr("Invalid data file (error while auto parsing for settings)"));
        }
    }

    /* Read complete file this time */
    if (bRet)
    {
        do
        {
            bRet = readLineFromFile(&line);

            if (bRet)
            {
                /* Check for color properties */
                QStringList colorList = line.split(_pAutoSettingsParser->fieldSeparator());

                if (static_cast<QString>(colorList.first()).toLower() == "//color")
                {
                    bool bValidColor;

                    // Remove color property name
                    colorList.removeFirst();

                    foreach(QString strColor, colorList)
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
            }
            else
            {
                showError(tr("Invalid data file (while reading comments)"));
                break;
            }

            lineIdx++;
        } while(lineIdx <= _pAutoSettingsParser->labelRow());
    }

    // Process register labels
    if (bRet)
    {
        pData->dataLabel.clear();
        pData->dataLabel.append(line.split(_pAutoSettingsParser->fieldSeparator()));
        _expectedFields = pData->dataLabel.size();

        if (_expectedFields <= 1)
        {
            showError(QString(tr("Incorrect graph data found. "
                                 "<br><br>Found field separator: \'%1\'")).arg(_pAutoSettingsParser->fieldSeparator()));

            bRet = false;
        }
    }

    /* Clear color list when size is not ok */
    if ((pData->colors.size() + 1) != (int)_expectedFields)
    {
         pData->colors.clear();
    }

    // Trim labels
    if (bRet)
    {
		// axis (time) label
        pData->axisLabel = pData->dataLabel[0].trimmed();

        for(qint32 i = 1; i < pData->dataLabel.size(); i++)
        {
            pData->dataLabel[i] = pData->dataLabel[i].trimmed();
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
                showError(tr("Invalid data file (while reading data from file)"));
            }

            lineIdx++;
        } while(lineIdx <= (qint32)_pAutoSettingsParser->dataRow());
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
            QStringList paramList = line.split(_pAutoSettingsParser->fieldSeparator());

            if (paramList.size() != (qint32)_expectedFields)
            {
                QString txt = QString(tr("The number of label columns doesn't match number of data columns!\nLabel count: %1\nData count: %2")).arg(_expectedFields).arg(paramList.size());
                showError(txt);
                bRet = false;
                break;
            }

            quint32 startColumn=1;
            if (_pAutoSettingsParser->absoluteDate())
            {
                bool bOk;
                const double number = (double)parseDateTime(paramList[0], &bOk);
                if (bOk)
                {
                    dataRows[0].append(number);
                }
                else
                {
                    QString error = QString(tr("Invalid absolute date (while processing data)\n"
                                               "Line: %1\n"
                                               "\n\nExpected decimal separator character: \'%2\'"
                                               ).arg(line).arg(_pAutoSettingsParser->locale().decimalPoint()));
                    showError(error);
                    bRet = false;
                    break;
                }
            }
            else
            {
                startColumn = 0;
            }

            for (qint32 i = startColumn; i < paramList.size(); i++)
            {
                bool bError = false;
                const double number = _pAutoSettingsParser->locale().toDouble(paramList[i], &bError);

                if (bError == false)
                {
                    QString error = QString(tr("Invalid data (while processing data)\n"
                                               "Line: %1\n"
                                               "\n\nExpected decimal separator character: \'%2\'"
                                               ).arg(line).arg(_pAutoSettingsParser->locale().decimalPoint()));
                    showError(error);
                    bRet = false;
                    break;
                }
                else
                {
                    dataRows[i].append(number);
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

void DataFileParser::loadDataFileSample(QStringList * pDataFileSample)
{
    QString lineData;
    pDataFileSample->clear();
    bool bRet = true;
    do
    {
        if (_pDataStream->atEnd())
        {
            break;
        }

        /* Read line */
        bRet = _pDataStream->readLineInto(&lineData, 0);

        if (bRet)
        {
            pDataFileSample->append(lineData);
        }
        else
        {
            pDataFileSample->clear();
            break;
        }

    } while(bRet && (pDataFileSample->size() < _cSampleLineLength));

    /* Set cursor back to beginning */
    _pDataStream->seek(0);

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

void DataFileParser::showError(QString text)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("ModbusScope data file load error"));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(text);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}


