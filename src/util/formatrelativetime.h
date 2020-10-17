#ifndef FORMATRELATIVETIME_H
#define FORMATRELATIVETIME_H

#include <QString>

namespace FormatRelativeTime
{

    QString timeStringFormat();
    QString formatTime(qint64 tickKey);
    QString formatTimeSmallScale(qint64 tickKey);
    QString formatTimeDiff(qint64 tickKeyDiff);

    bool IsDateRelative(qint64 ticks);

}

#endif // FORMATRELATIVETIME_H
