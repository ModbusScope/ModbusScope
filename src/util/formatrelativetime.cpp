
#include "formatrelativetime.h"
#include "formatdatetime.h"

static const quint32 cSecondsInADay = 24 * 60 * 60 * 1000;

QString FormatRelativeTime::timeStringFormat()
{
    return QString("HH:mm:ss%1zzz").arg(QLocale::system().decimalPoint());
}

QString FormatRelativeTime::formatTime(qint64 tickKey)
{
    QString tickLabel;
    bool bNegative;
    quint64 absoluteTime;

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

    if (FormatRelativeTime::IsDateRelative(tickKey))
    {
        /* Round number to a day */
        absoluteTime %= cSecondsInADay;

        QTime time = QTime::fromMSecsSinceStartOfDay(absoluteTime);

        tickLabel = time.toString(timeStringFormat());
    }
    else
    {
        /* Absolute date */
        QDateTime dateTime;
        dateTime.setMSecsSinceEpoch(tickKey); /* Converts from ms since epoch in UTC to local timezone */

        QString timeStringFormat = QString("%1\n%2").arg(FormatDateTime::dateStringFormat()).arg(FormatDateTime::timeStringFormat());
        tickLabel = dateTime.toString(timeStringFormat);
    }

    // Make sure minus sign is shown when tick number is negative
    if (bNegative)
    {
        tickLabel = "-" + tickLabel;
    }

    return tickLabel;
}

QString FormatRelativeTime::formatTimeSmallScale(qint64 tickKey)
{
    return QString("%1").arg(tickKey);
}

QString FormatRelativeTime::formatTimeDiff(qint64 tickKeyDiff)
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

    if (tickKeyDiff < cSecondsInADay)
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
            tickLabel = timeDiff.toString(timeStringFormat());
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

bool FormatRelativeTime::IsDateRelative(qint64 ticks)
{
    bool bRet = false;

    if (ticks < QDateTime::fromString("2000-01-01", Qt::ISODate).toMSecsSinceEpoch())
    {
        bRet = true;
    }

    return bRet;
}
