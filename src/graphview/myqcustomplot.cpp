#include "QWidget"
#include "QDebug"

#include "myqcustomplot.h"
#include "myqcpgraph.h"

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

/* Return our custom QCPGraph object */
MyQCPGraph *MyQCustomPlot::graph(int index) const
{
  if (index >= 0 && index < mGraphs.size())
  {
    return (MyQCPGraph *)mGraphs.at(index);
  } else
  {
    qDebug() << Q_FUNC_INFO << "index out of bounds:" << index;
    return 0;
  }
}

/* Add our custom QCPGraph instead of default QCPGraph */
MyQCPGraph * MyQCustomPlot::addCustomGraph()
{

  QCPGraph *newGraph = new MyQCPGraph(xAxis, yAxis);
  if (addPlottable(newGraph))
  {
    newGraph->setName(QLatin1String("Graph ")+QString::number(mGraphs.size()));
    return (MyQCPGraph *)newGraph;
  } else
  {
    delete newGraph;
    return 0;
  }
}
