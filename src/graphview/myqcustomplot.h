#ifndef MYQCUSTOMPLOT_H
#define MYQCUSTOMPLOT_H

#include <QObject>
#include "qcustomplot.h"
#include "myqcpgraph.h"

class MyQCustomPlot : public QCustomPlot
{
public:
    MyQCustomPlot();
    MyQCustomPlot(QWidget *parent);
    ~MyQCustomPlot();

    virtual void enterEvent(QEvent * event);

    MyQCPGraph *graph(int index) const;
    MyQCPGraph *addCustomGraph();
};

#endif // MYQCUSTOMPLOT_H
