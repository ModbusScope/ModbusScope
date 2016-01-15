#include "QWidget"
#include "QDebug"

#include "myqcpgraph.h"

#include "myqcustomplot.h"


/*
 * Custom QCPGraph class because we want to manage the data ourselves
 * The standard QCPGraph class takes ownership of the datamap. Our data(map) is managed by the model so this subclass doesn't take ownership of the datamap object
 *
 * Some dirty stuff is introduced to make sure everything works
 *
 * The original constructor is always called, an object is always created. When we assign a pointer, we need to delete this pointer. This is solved by the bInternalData variable.
 * The original destructor is always called, the base destructor always delete the object. This is solved by replacing the pointer with a new object in our destructor.
 */


MyQCPGraph::MyQCPGraph(QCPAxis *keyAxis, QCPAxis *valueAxis) : QCPGraph(keyAxis, valueAxis)
{
    // Base constructor is called
    bInternalData = true;
}

MyQCPGraph::~MyQCPGraph()
{
    if (!bInternalData)
    {
        // DIRTY
        // Make sure destructor for base class doesn't delete external data
        mData = new QCPDataMap;
    }
}

void MyQCPGraph::setData(QCPDataMap *data, bool copy)
{
    if (mData == data)
    {
        //qDebug() << Q_FUNC_INFO << "The data pointer is already in (and owned by) this plottable" << reinterpret_cast<quintptr>(data);
        return;
    }
    if (copy)
    {
        *mData = *data;
    }
    else
    {
        // Only delete when internal (init) instance is used
        if (bInternalData) // delete mData; // This is a change
        {
            bInternalData = false;
            delete mData;
        }

        mData = data;
    }
}
