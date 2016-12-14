#ifndef MYQCPAXIS_H
#define MYQCPAXIS_H

#include <QObject>
#include "qcustomplot.h"

class MyQCPAxis : public QCPAxis
{
public:
    MyQCPAxis(QCPAxisRect *parent, AxisType type);
    void rescaleValue(QCPRange keyAxisRange);

    void setRange(double lower, double upper);

public slots:
    void setRange(const QCPRange &range);

private:
    void addMargin(double * pLower, double * pUpper);

};

#endif // MYQCPAXIS_H
