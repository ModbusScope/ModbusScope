#ifndef VALUEAXIS_H
#define VALUEAXIS_H

#include <QObject>
#include "qcustomplot.h"

class ValueAxis : public QCPAxis
{
public:
    ValueAxis(QCPAxisRect *parent, AxisType type);
    void rescaleValue(QCPRange keyAxisRange);

    void setRange(double lower, double upper);

public slots:
    void setRange(const QCPRange &range);

private:
    void addMargin(double * pLower, double * pUpper);

};

#endif // VALUEAXIS_H
