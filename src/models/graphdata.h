#ifndef GRAPHDATA_H
#define GRAPHDATA_H

#include "models/graphdataseries.h"

#include <QColor>
#include <QSharedPointer>
#include <QtGlobal>

/*!
 * \brief Settings and data of a single graph.
 *
 * Note: copies of a GraphData share the same underlying GraphDataSeries
 * (GraphDataStore relies on this when storing GraphData by value).
 */
class GraphData
{

public:
    explicit GraphData();
    ~GraphData();

    enum class ExpressionState
    {
        UNKNOWN = 0,
        VALID,
        SYNTAX_ERROR,
        UNKNOWN_DEVICE
    };

    typedef enum
    {
        VALUE_AXIS_PRIMARY = 0,
        VALUE_AXIS_SECONDARY,
        VALUE_AXIS_CNT
    } valueAxis_t;

    valueAxis_t valueAxis() const;
    void setValueAxis(valueAxis_t axis);

    bool isVisible() const;
    void setVisible(bool bVisible);

    QString label() const;
    void setLabel(const QString& label);

    QColor color() const;
    void setColor(const QColor& color);

    bool isActive() const;
    void setActive(bool bActive);

    QString expression() const;
    void setExpression(QString expression);

    ExpressionState expressionState() const;
    void setExpressionState(ExpressionState status);
    bool isExpressionValid() const;

    QSharedPointer<const GraphDataSeries> dataSeries() const;
    QSharedPointer<GraphDataSeries> mutableDataSeries();

private:
    valueAxis_t _valueAxis;
    bool _bVisible;
    QString _label;
    QColor _color;

    bool _bActive;
    QString _expression;
    ExpressionState _expressionState;

    QSharedPointer<GraphDataSeries> _pDataSeries;
};

#endif // GRAPHDATA_H
