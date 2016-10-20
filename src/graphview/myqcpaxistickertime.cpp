#include "util.h"

#include "myqcpaxistickertime.h"

MyQCPAxisTickerTime::MyQCPAxisTickerTime(QCustomPlot * pPlot)
{
    _pPlot = pPlot;

    setTimeFormat("day %d\n%h:%m:%s:%z");
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


#if 0
    // TODO
    QVector<double> ticks = _pPlot->xAxis->tickVector();

    /* Clear ticks vector */
    tickLabels.clear();

    const bool bSmallScale = smallScaleActive(ticks);

    /* Generate correct labels */
    for (qint32 index = 0; index < ticks.size(); index++)
    {
        tickLabels.append(Util::formatTime(ticks[index], bSmallScale));
    }

    /* Set labels */
    _pPlot->xAxis->setTickVectorLabels(tickLabels);
#endif

#if 0
    bool BasicGraphView::smallScaleActive(double begin, double end)
    {
        bool bRet = false;
        if (qAbs(end - begin) < _cSmallScaleDiff)
        {
            bRet = true;
        }

        if (bRet)
        {
            _pPlot->xAxis->setLabel("Time (ms)");
        }
        else
        {
            _pPlot->xAxis->setLabel("Time (s)");
        }

        return bRet;
    }
#endif


//_pPlot->xAxis->setLabel("Time");
//timeTicker->setTimeFormat("day %d\n%h:%m:%s:%z");


