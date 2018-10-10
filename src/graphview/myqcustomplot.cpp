#include "QWidget"

#include "myqcustomplot.h"

MyQCustomPlot::MyQCustomPlot()
{

}

MyQCustomPlot::MyQCustomPlot(QWidget *parent):
    QCustomPlot(parent)
{

}

MyQCustomPlot::~MyQCustomPlot()
{

}

void MyQCustomPlot::enterEvent(QEvent * event)
{
    if (
        (cursor().shape() == Qt::SizeVerCursor)
        || (cursor().shape() == Qt::SizeHorCursor)
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
