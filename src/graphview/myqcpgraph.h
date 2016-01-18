#ifndef MYQCUSTOMGRAPH_H
#define MYQCUSTOMGRAPH_H

#include <QObject>
#include "qcustomplot.h"

class MyQCPGraph : public QCPGraph
{
public:
    explicit MyQCPGraph(QCPAxis *keyAxis, QCPAxis *valueAxis);
    ~MyQCPGraph();

    using QCPGraph::setData;
    void setData(QCPDataMap *data, bool copy);

private:
    bool bInternalData;
};

#endif // MYQCUSTOMGRAPH_H
