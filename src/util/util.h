#ifndef UTIL_H
#define UTIL_H

#include <QObject>
#include <QMessageBox>
#include <QLocale>
#include <QColor>
#include <QDateTime>

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
        msgBox.setWindowTitle(tr("GraphViewer"));
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

        // Round number to 3 decimals and remove group separator
        tmp = QLocale::system().toString(number, 'f', 3).replace(QLocale::system().groupSeparator(), "");

        // convert back to double
        double doubleTmp = QLocale::system().toDouble(tmp);

        // Make sure trailing zeros are removed and remove group separator
        tmp = QLocale::system().toString(doubleTmp, 'g', 9).replace(QLocale::system().groupSeparator(), "");

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
                dateTime.setMSecsSinceEpoch(tickKey);
                tickLabel = dateTime.toString("dd/MM/yyyy \n" + Util::timeStringFormat());
            }
            else
            {
                /* Round number to a day */
                absoluteTime %= Util::cSecondsInADay;

                QTime time = QTime::fromMSecsSinceStartOfDay(absoluteTime);

                tickLabel = time.toString();
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
