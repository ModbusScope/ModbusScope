
#include "formatrelativetime.h"
#include "formatdatetime.h"

#include <QtMath>

namespace FormatRelativeTime
{
    const quint32 cSecondsInADay = 24 * 60 * 60 * 1000;

    enum {
        MSECS_PER_SECS = 1000,
        SECS_PER_MINUTE = 60,
        MINS_PER_HOUR = 60,
    };
}

QString FormatRelativeTime::timeStringFormat()
{
    return QString("HH:mm:ss%1zzz").arg(QLocale().decimalPoint());
}

QString FormatRelativeTime::formatTime(qint64 tickKey)
{
    QString tickLabel;
    bool bNegative;
    quint64 absoluteTick;

    if (tickKey < 0)
    {
        bNegative = true;
    }
    else
    {
        bNegative = false;

    }
    absoluteTick = qFabs(tickKey);

    if (FormatRelativeTime::IsDateRelative(absoluteTick))
    {
        quint32 msecs;
        quint32 secs;
        quint32 mins;
        quint32 hours;

        quint64 ticks = absoluteTick;

        msecs = ticks % MSECS_PER_SECS;
        ticks /= MSECS_PER_SECS;

        secs = ticks % SECS_PER_MINUTE;
        ticks /= SECS_PER_MINUTE;

        mins = ticks % MINS_PER_HOUR;
        ticks /= MINS_PER_HOUR;

        hours = ticks;

        tickLabel = QString(QStringLiteral("%1:%2:%3%4%5"))
                        .arg(hours, 2, 10, QLatin1Char('0'))
                        .arg(mins, 2, 10, QLatin1Char('0'))
                        .arg(secs, 2, 10, QLatin1Char('0'))
                        .arg(QLocale().decimalPoint()) /* Decimal point */
                        .arg(msecs, 3, 10, QLatin1Char('0'));
    }
    else
    {
        /* Absolute date */
        QDateTime dateTime;
        dateTime.setMSecsSinceEpoch(absoluteTick); /* Converts from ms since epoch in UTC to local timezone */

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
            QString secondStringFormat = QString("ss%1zzz").arg(QLocale().decimalPoint());
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
