#ifndef MYQCPAXISTICKERTIME_H
#define MYQCPAXISTICKERTIME_H

#include <QObject>
#include "qcustomplot.h"

class MyQCPAxisTickerTime : public QCPAxisTickerTime
{
public:
    MyQCPAxisTickerTime(QCustomPlot *pPlot);

    QString getTickLabel(double tick, const QLocale & locale, QChar formatChar, int precision );

private:
    QCustomPlot *_pPlot;

    static const quint32 _cSmallScaleDiff = 2000;
};

#endif // MYQCPAXISTICKERTIME_H
