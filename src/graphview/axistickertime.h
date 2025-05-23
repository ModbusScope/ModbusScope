#ifndef AXISTICKERTIME_H
#define AXISTICKERTIME_H

#include <QObject>
#include "qcustomplot/qcustomplot.h"

class AxisTickerTime : public QCPAxisTickerTime
{
public:
    AxisTickerTime(QCustomPlot *pPlot);

    QString getTickLabel(double tick, const QLocale & locale, QChar formatChar, int precision );

private:
    QCustomPlot *_pPlot;

    static const quint32 _cSmallScaleDiff = 2000;
};

#endif // AXISTICKERTIME_H
