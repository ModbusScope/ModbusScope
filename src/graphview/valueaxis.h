#ifndef VALUEAXIS_H
#define VALUEAXIS_H

#include <QObject>
#include "qcustomplot/qcustomplot.h"

class ValueAxis : public QCPAxis
{
public:
    ValueAxis(QCPAxisRect *parent, AxisType type);
    void rescaleValue(QCPRange keyAxisRange);

};

#endif // VALUEAXIS_H
