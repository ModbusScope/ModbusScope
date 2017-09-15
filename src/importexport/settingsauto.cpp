#include <QFile>
#include <QLocale>
#include <QRegularExpression>

#include "settingsauto.h"
#include "util.h"

SettingsAuto::SettingsAuto()
{
}

bool SettingsAuto::updateSettings(QStringList previewData)
{
    bool bRet = true;

    // Find first non-comment line
    qint32 lineIdx = 0;
    lineIdx = nextDataLine(0, previewData, &bRet);

    if (bRet)
    {
        _labelRow = lineIdx;

        // find next dataline
        lineIdx++;
        lineIdx = nextDataLine(lineIdx, previewData, &bRet);
    }

    if (bRet)
    {
        // Set data row
        _dataRow = lineIdx;

        /*
         * Different locales
         * ------------------
         * French (fr_FR):      4 294 967 295,000
         * Italian (it_IT):     4.294.967.295,000
         * US-English (en_US):  4,294,967,295.00
         */
        QLocale frLocale(QLocale("fr_FR"));
        QLocale itLocale(QLocale("it_IT"));
        QLocale enLocale(QLocale("en_US"));

        // First try French locale
        bool bParse;
        bParse = testLocale(previewData, frLocale, ';');

        if (!bParse)
        {
            // Else try Italian locale
            bParse = testLocale(previewData, itLocale, ';');

            if (!bParse)
            {
                // Else try US English locale
                bRet = testLocale(previewData, enLocale, ',');
            }
        }
    }
    else
    {
        bRet = false;
    }

    // Check for absolute dates
    if (bRet)
    {
        _bAbsoluteDate = testAbsoluteDate(previewData);
    }

    return bRet;
}


QChar SettingsAuto::fieldSeparator()
{
    return _fieldSeparator;
}

QString SettingsAuto::commentSequence()
{
    return _commentSequence;
}

quint32 SettingsAuto::dataRow()
{
    return _dataRow;
}

qint32 SettingsAuto::labelRow()
{
    return _labelRow;
}

QLocale SettingsAuto::locale()
{
    return _locale;
}

bool SettingsAuto::absoluteDate()
{
    return _bAbsoluteDate;
}

bool SettingsAuto::isComment(QString line)
{
    bool bRet = false;
    if (_commentSequence.isEmpty())
    {
        // Check first character for comment char
        if (!line.at(0).isLetterOrNumber())
        {
            // Check second character
            if (
                (!line.at(1).isLetterOrNumber())
                && (!line.at(1).isSpace())
                )
            {
                _commentSequence = line.left(2);
                bRet = true;
            }
            else
            {
                _commentSequence = line.left(1);
                bRet = true;
            }
        }
    }
    else
    {
        if (line.left(_commentSequence.size()) == _commentSequence)
        {
            bRet = true;
        }
    }

    return bRet;
}

bool SettingsAuto::isAbsoluteDate(QString rawData)
{
    QRegularExpression re("\\d{2,4}.*\\d{2}.*\\d{2,4}\\s*\\d{1,2}:\\d{1,2}:\\d{1,2}.*");
    QRegularExpressionMatch match = re.match(rawData);

    return match.hasMatch();
}

bool SettingsAuto::testLocale(QStringList previewData, QLocale locale, QChar fieldSeparator)
{
    quint32 aSeparatorCount[2];
    aSeparatorCount[0] = previewData[_labelRow].count(fieldSeparator);
    aSeparatorCount[1] = previewData[_dataRow].count(fieldSeparator);

    if (
            (aSeparatorCount[0] == aSeparatorCount[1])
            && (aSeparatorCount[0] > 0)
        )
    {
        qint32 parseIdx;
        for (parseIdx = _dataRow; parseIdx < previewData.size(); parseIdx++)
        {
            if (!isComment(previewData[parseIdx]))
            {

                QStringList fields = previewData[parseIdx].split(fieldSeparator);

                for (qint32 idx = 1; idx < fields.size(); idx++)
                {

                    bool bOk;
                    locale.toDouble(fields[idx], &bOk);
                    if (!bOk)
                    {
                        return false;
                    }
                }                
            }
        }
    }
    else
    {
        return false;
    }

    // Save locale
    _locale = locale;

    // save field separator
    _fieldSeparator = fieldSeparator;

    return true;
}

bool SettingsAuto::testAbsoluteDate(QStringList previewData)
{
    qint32 parseIdx;
    for (parseIdx = _dataRow; parseIdx < previewData.size(); parseIdx++)
    {
        if (!isComment(previewData[parseIdx]))
        {

            QStringList fields = previewData[parseIdx].split(_fieldSeparator);
            QString timeOrDate = fields[0];

            if (isAbsoluteDate(timeOrDate))
            {
                // Check next
            }
            else
            {
                // Not valid
                return false;
            }
        }
    }

    // absolute date
    return true;
}

quint32 SettingsAuto::nextDataLine(quint32 startIdx, QStringList previewData, bool *bOk)
{
    qint32 lineIdx;
    for (lineIdx = startIdx; lineIdx < previewData.size(); lineIdx++)
    {
        QString line = previewData[lineIdx].trimmed();
        if (!line.isEmpty())
        {
            if (!isComment(line))
            {
                break;
            }
        }
    }

    if (lineIdx < previewData.size())
    {
        *bOk = true;
    }
    else
    {
        *bOk = false;
    }

    return lineIdx;
}


