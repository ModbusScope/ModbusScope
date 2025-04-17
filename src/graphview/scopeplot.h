#ifndef SCOPEPLOT_H
#define SCOPEPLOT_H

#include "qcustomplot/qcustomplot.h"
#include <QObject>

class ScopePlot : public QCustomPlot
{
public:
    explicit ScopePlot(QWidget *parent);
    ~ScopePlot();

    virtual void enterEvent(QEnterEvent * event);

};

#endif // SCOPEPLOT_H
