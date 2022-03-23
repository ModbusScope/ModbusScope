#ifndef GRAPHDATA_H
#define GRAPHDATA_H

#include <QtGlobal>
#include <QColor>
#include "qcustomplot.h"

class GraphData
{

public:
    explicit GraphData();
    ~GraphData();

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
    void setLabel(const QString &label);

    QColor color() const;
    void setColor(const QColor &color);

    bool isActive() const;
    void setActive(bool bActive);

    QString expression() const;
    void setExpression(QString expression);

    QSharedPointer<QCPGraphDataContainer> dataMap();

private:

    valueAxis_t _valueAxis;
    bool _bVisible;
    QString _label;
    QColor _color;

    bool _bActive;
    QString _expression;

    QSharedPointer<QCPGraphDataContainer> _pDataMap;

};

#endif // GRAPHDATA_H
