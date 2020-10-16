#include "formatrelativetime.h"

#include "myqcpaxistickertime.h"

MyQCPAxisTickerTime::MyQCPAxisTickerTime(QCustomPlot * pPlot)
{
    _pPlot = pPlot;
}

QString MyQCPAxisTickerTime::getTickLabel(double tick, const QLocale & locale, QChar formatChar, int precision )
{
    Q_UNUSED(locale);
    Q_UNUSED(formatChar);
    Q_UNUSED(precision);

    QString tickLabel;

    if (
            (_pPlot->xAxis->range().size() <= _cSmallScaleDiff)
            && (_pPlot->xAxis->range().upper < QDateTime::fromString("2000-01-01", Qt::ISODate).toMSecsSinceEpoch())
        )
    {
        tickLabel = FormatRelativeTime::formatTime(tick, true);
    }
    else
    {
        tickLabel = FormatRelativeTime::formatTime(tick, false);
    }

    return tickLabel;
}
