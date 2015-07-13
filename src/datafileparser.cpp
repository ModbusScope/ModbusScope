
#include <QtWidgets>
#include <QMessageBox>
#include "datafileparser.h"
#include "util.h"

DataFileParser::DataFileParser(QFile *pDataFile)
{
    _pDataFile = pDataFile;
}

// Return false on error
bool DataFileParser::processDataFile(FileData * pData)
{
    QString line;
    bool bRet = true;

    /*-- Read until real data --*/
    qint32 state = 0; /* -1: error or end of file, 0: continue reading, 1 = start of data */
    do
    {
        bool bResult = readLineFromFile(&line);

        if (!bResult)
        {
            state = -1;
        }
        else if (line.size() == 1)
        {
            // Empty line, so skip line
        }
        else
        {
            if (line.left(2) == "//")
            {
                // Comment line, so skip line
            }
            else
            {
                // Ok first non comment and not empty line
                state = 1;
            }
        }
    } while (state == 0);


    // Next data line is already read

    if (state == -1)
    {        
        showError(tr("Invalid data file (while reading comments)"));
        bRet = false;
    }

    // Process register labels
    if (bRet)
    {
        pData->dataLabel.clear();
        pData->dataLabel.append(line.split(Util::separatorCharacter()));
        _expectedFields = pData->dataLabel.size();
    }

    if (_expectedFields <= 1)
    {
        showError(QString(tr("No graph data is found. "
                             "Are you sure the separator character is according to your locale?"
                             "<br><br>Expected separator: \'%1\'")).arg(Util::separatorCharacter()));

        bRet = false;
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
        QStringList paramList = line.split(Util::separatorCharacter());

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
            const double number = QLocale::system().toDouble(paramList[i], &bError);

            if (bError == false)
            {
                QString error = QString(tr("Invalid data (while processing data)\n"
                                           "Line: %1\n"
                                           "Are you sure the decimal separator character is according to your locale?"
                                           "\n\nExpected decimal separator: \'%2\'"
                                           ).arg(line).arg(Util::separatorCharacter()));
                showError(error);
                bRet = false;
                break;
            }
            else
            {
                dataRows[i].append(number);
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

void DataFileParser::showError(QString text)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("ModbusScope data file load error"));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(text);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}


