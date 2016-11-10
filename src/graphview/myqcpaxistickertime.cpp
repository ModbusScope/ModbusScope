#include "util.h"

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

    if (_pPlot->xAxis->range().size() <= _cSmallScaleDiff)
    {
        tickLabel = Util::formatTime(tick, true);
    }
    else
    {
        tickLabel = Util::formatTime(tick, false);
    }

    return tickLabel;
}
