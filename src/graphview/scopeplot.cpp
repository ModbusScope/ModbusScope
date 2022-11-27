#include <QWidget>

#include "scopeplot.h"

ScopePlot::ScopePlot()
{

}

ScopePlot::ScopePlot(QWidget *parent):
    QCustomPlot(parent)
{

}

ScopePlot::~ScopePlot()
{

}

void ScopePlot::enterEvent(QEnterEvent *event)
{
    if (
        (cursor().shape() == Qt::SizeVerCursor)
        || (cursor().shape() == Qt::SizeBDiagCursor)
        || (cursor().shape() == Qt::SizeFDiagCursor)
        || (cursor().shape() == Qt::SizeHorCursor)
        || (cursor().shape() == Qt::SplitHCursor)
        || (cursor().shape() == Qt::SplitVCursor)
     )
    {
        this->setCursor(Qt::ArrowCursor);
    }

    QCustomPlot::enterEvent(event);
}
