#ifndef SCOPEPLOT_H
#define SCOPEPLOT_H

#include <QObject>
#include "qcustomplot.h"

class ScopePlot : public QCustomPlot
{
public:
    ScopePlot();
    ScopePlot(QWidget *parent);
    ~ScopePlot();

    virtual void enterEvent(QEnterEvent * event);

};

#endif // SCOPEPLOT_H
