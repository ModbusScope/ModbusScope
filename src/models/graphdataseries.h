#ifndef GRAPHDATASERIES_H
#define GRAPHDATASERIES_H

#include <QList>

//! Single sample of a graph: timestamp in milliseconds and the computed value
struct GraphSample
{
    double timestamp;
    double value;
};

class GraphDataSeries
{
public:
    typedef QList<GraphSample>::iterator iterator;
    typedef QList<GraphSample>::const_iterator const_iterator;

    //! Timestamp range of a data series
    struct TimestampRange
    {
        double min;
        double max;
    };

    GraphDataSeries();

    qsizetype size() const;
    bool isEmpty() const;
    void clear();

    void add(const GraphSample& sample);
    void add(double timestamp, double value);
    void setSamples(const QList<double>& timestamps, const QList<double>& values);

    iterator begin();
    iterator end();
    const_iterator constBegin() const;
    const_iterator constEnd() const;

    const_iterator findBegin(double timestamp, bool expandedRange = true) const;
    const_iterator findEnd(double timestamp, bool expandedRange = true) const;

    TimestampRange timestampRange(bool& bValid) const;

private:
    QList<GraphSample> _samples;
};

#endif // GRAPHDATASERIES_H
