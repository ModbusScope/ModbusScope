#include "QWidget"
#include "QDebug"

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
        (cursor().shape() == Qt::SplitVCursor)
        || (cursor().shape() == Qt::SplitVCursor)
     )
    {
        this->setCursor(Qt::ArrowCursor);
        //cursor().setShape(Qt::ArrowCursor);
        qDebug() << "Changed";
    }

    QCustomPlot::enterEvent(event);
}
