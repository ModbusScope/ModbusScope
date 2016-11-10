#ifndef MYQCUSTOMPLOT_H
#define MYQCUSTOMPLOT_H

#include <QObject>
#include "qcustomplot.h"

class MyQCustomPlot : public QCustomPlot
{
public:
    MyQCustomPlot();
    MyQCustomPlot(QWidget *parent);
    ~MyQCustomPlot();

    virtual void enterEvent(QEvent * event);

};

#endif // MYQCUSTOMPLOT_H
