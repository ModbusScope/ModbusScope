#ifndef GRAPHDATASERIES_H
#define GRAPHDATASERIES_H

#include "util/result.h"

#include <QList>

//! Single sample of a signal: timestamp in milliseconds, the computed value, and its data quality
struct GraphSample
{
    double timestamp;
    double value;
    DataQuality::Quality quality{};
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
    void add(double timestamp, double value, const DataQuality::Quality& quality);
    void setSamples(const QList<double>& timestamps, const QList<double>& values);
    void setSamples(const QList<double>& timestamps,
                    const QList<double>& values,
                    const QList<DataQuality::Quality>& qualities);

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
