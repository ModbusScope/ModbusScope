#ifndef MYQCPAXIS_H
#define MYQCPAXIS_H

#include <QObject>
#include "qcustomplot.h"

class MyQCPAxis : public QCPAxis
{
public:
    MyQCPAxis(QCPAxisRect *parent, AxisType type);

    void rescaleValue(QCPRange keyAxisRange);
};

#endif // MYQCPAXIS_H
