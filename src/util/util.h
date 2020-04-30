#ifndef UTIL_H
#define UTIL_H

#include <QObject>
#include <QMessageBox>
#include <QLocale>
#include <QColor>
#include <QDateTime>
#include "version.h"

class Util : public QObject
{
    Q_OBJECT

public:

    static QChar separatorCharacter()
    {
        if (QLocale::system().decimalPoint() == ',')
        {
            return ';';
        }
        else
        {
            return ',';
        }
    }

    static void showError(QString text)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("ModbusScope"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(text);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }

    static const QString & currentVersion()
    {
        static const QString version(APP_VERSION);
        return version;
    }

    static QString formatDoubleForExport(double number)
    {
        QString tmp;
        
        /* TODO: Can't this be done without converting to string and back and back again? */

        // Round number to 3 decimals
        tmp = QLocale::system().toString(number, 'f', 3);

        // Remove group separator
        if (QLocale::system().groupSeparator() != QLocale::system().decimalPoint())
        {
            tmp = tmp.replace(QLocale::system().groupSeparator(), "");
        }

        // convert back to double
        double doubleTmp = QLocale::system().toDouble(tmp);

        // Make sure trailing zeros are removed
        tmp = QLocale::system().toString(doubleTmp, 'g', 9);

        // Remove group separator
        if (QLocale::system().groupSeparator() != QLocale::system().decimalPoint())
        {
            tmp = tmp.replace(QLocale::system().groupSeparator(), "");
        }

        return tmp;
    }

    static QString timeStringFormat()
    {
        return QString("HH:mm:ss%1zzz").arg(QLocale::system().decimalPoint());
    }

    static QString formatTime(qint64 tickKey, bool bSmallScale)
    {
        QString tickLabel;
        bool bNegative;
        quint64 absoluteTime;

        if (bSmallScale)
        {
            tickLabel = QString("%1").arg(tickKey);
        }
        else
        {
            if (tickKey < 0)
            {
                bNegative = true;
                absoluteTime = -1 * tickKey;
            }
            else
            {
                bNegative = false;
                absoluteTime = tickKey;
            }

            if (tickKey > QDateTime::fromString("2000-01-01", Qt::ISODate).toMSecsSinceEpoch())
            {
                /* Absolute date */
                QDateTime dateTime;
                dateTime.setMSecsSinceEpoch(tickKey); /* Converts from ms since epoch in UTC to local timezone */
                tickLabel = dateTime.toString("dd/MM/yyyy \n" + Util::timeStringFormat());
            }
            else
            {
                /* Round number to a day */
                absoluteTime %= Util::cSecondsInADay;

                QTime time = QTime::fromMSecsSinceStartOfDay(absoluteTime);

                tickLabel = time.toString(Util::timeStringFormat());
            }

            // Make sure minus sign is shown when tick number is negative
            if (bNegative)
            {
                tickLabel = "-" + tickLabel;
            }
        }

        return tickLabel;
    }

    static QString formatTimeDiff(qint64 tickKeyDiff)
    {
        QString tickLabel;
        bool bNegative;
        quint64 absoluteDiff;

        if (tickKeyDiff < 0)
        {
            bNegative = true;
            absoluteDiff = -1 * tickKeyDiff;
        }
        else
        {
            bNegative = false;
            absoluteDiff = tickKeyDiff;
        }

        if (tickKeyDiff < Util::cSecondsInADay)
        {
            QTime timeDiff = QTime::fromMSecsSinceStartOfDay(absoluteDiff);
            if (absoluteDiff < 60000) /* Under a minute */
            {
                /* Use short time diff notation: seconds and milliseconds */
                QString secondStringFormat = QString("ss%1zzz").arg(QLocale::system().decimalPoint());
                tickLabel = timeDiff.toString(secondStringFormat);
            }
            else
            {
                /* Use full time diff notation: hour,seconds and milliseconds */
                tickLabel = timeDiff.toString(Util::timeStringFormat());
            }

            // Make sure minus sign is shown when tick number is negative
            if (bNegative)
            {
                tickLabel = "-" + tickLabel;
            }
        }
        else
        {
            tickLabel = QString("Difference is too large!");
        }



        return tickLabel;
    }
    
    static const QList<QColor> cColorlist;
    static const quint32 cSecondsInADay = 24 * 60 * 60 * 1000;

private:

};


#endif // UTIL_H
