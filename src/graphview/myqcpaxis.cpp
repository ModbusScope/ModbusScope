#include "myqcpaxis.h"
#include "QDebug"


MyQCPAxis::MyQCPAxis(QCPAxisRect *parent, AxisType type):
    QCPAxis(parent, type)
{

}

/*!
  Rescales the value axis of all the plottables so all whole plottables are visible.
*/
void MyQCPAxis::rescaleValue(QCPRange keyAxisRange)
{
    QList<QCPAbstractPlottable*> p = plottables();
    QCPRange newRange;
    bool haveRange = false;

    for (int i = 0; i < p.size(); ++i)
    {
        if (!p.at(i)->realVisibility())
        {
            continue;
        }

        QCPRange plottableRange;
        bool currentFoundRange;
        plottableRange = p.at(i)->getValueRange(currentFoundRange, QCP::sdBoth, keyAxisRange);

        if (currentFoundRange)
        {
            if (!haveRange)
            {
                newRange = plottableRange;
            }
            else
            {
                newRange.expand(plottableRange);
            }
            haveRange = true;
        }
    }

    if (haveRange)
    {
        if (!QCPRange::validRange(newRange)) // likely due to range being zero (plottable has only constant data in this axis dimension), shift current range to at least center the plottable
        {
            double center = (newRange.lower+newRange.upper)*0.5; // upper and lower should be equal anyway, but just to make sure, incase validRange returned false for other reason
            newRange.lower = center-mRange.size()/2.0;
            newRange.upper = center+mRange.size()/2.0;
        }
        setRange(newRange);
    }
}

void MyQCPAxis::setRange(const QCPRange &range)
{
    double newLower = range.lower;
    double newUpper = range.upper;

    addMargin(&newLower, &newUpper);

    QCPAxis::setRange(QCPRange(newLower, newUpper));
}

void MyQCPAxis::setRange(double lower, double upper)
{
    double newLower = lower;
    double newUpper = upper;

    addMargin(&newLower, &newUpper);

    QCPAxis::setRange(newLower, newUpper);
}

void MyQCPAxis::addMargin(double * pLower, double * pUpper)
{
    if (graphs().size() > 0)
    {
        const double diff = pixelToCoord(0) - pixelToCoord(graphs()[0]->scatterStyle().size());

        *pLower += diff;
        *pUpper += diff;
    }
}
