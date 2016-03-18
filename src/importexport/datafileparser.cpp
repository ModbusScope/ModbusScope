
#include <QtWidgets>
#include <QMessageBox>
#include "datafileparser.h"

DataFileParser::DataFileParser()
{
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

            if (!bRet)
            {
                showError(tr("Invalid data file (while reading comments)"));
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

            for (qint32 i = 0; i < paramList.size(); i++)
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

void DataFileParser::showError(QString text)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("ModbusScope data file load error"));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(text);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}


