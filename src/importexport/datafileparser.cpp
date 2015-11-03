
#include <QtWidgets>
#include <QMessageBox>
#include "datafileparser.h"

DataFileParser::DataFileParser(QFile *pDataFile)
{
    _pDataFile = pDataFile;

    loadDataFileSample();

    _pDataFile->seek(0);

    _pAutoSettingsParser = new SettingsAuto();
}

DataFileParser::~DataFileParser()
{
    delete _pAutoSettingsParser;
}

// Return false on error
bool DataFileParser::processDataFile(FileData * pData)
{
    QString line;
    qint32 lineIdx = 0;
    bool bRet = true;

    bRet = _pAutoSettingsParser->updateSettings(_dataFileSample);

    if (!bRet)
    {
        showError(tr("Invalid data file (error while auto parsing for settings)"));
        bRet = false;
    }

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
        if (_pDataFile->atEnd())
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
    bool bRet = false;
    char buf[2048];
    qint32 lineLength;

    // Read first line of data (labels)
    lineLength = _pDataFile->readLine(buf, sizeof(buf));
    if (lineLength > 0)
    {
        bRet = true;
        *pLine = QString(buf);
    }

    return bRet;

}

void DataFileParser::loadDataFileSample()
{
    char buf[2048];
    qint32 lineLength;

    _dataFileSample.clear();

    while((!_pDataFile->atEnd()) && (_dataFileSample.size() < _cSampleLineLength))
    {
        lineLength = _pDataFile->readLine(buf, sizeof(buf));
        if (lineLength > -1)
        {
            _dataFileSample.append(QString(buf));
        }
        else
        {
            _dataFileSample.clear();
            break;
        }
    }
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


