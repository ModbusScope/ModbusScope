#include "graphdata.h"

#include "util/util.h"

GraphData::GraphData()
{
    _valueAxis = VALUE_AXIS_PRIMARY;
    _bVisible = true;
    _label = QString("Unknown register");
    _color = "-1"; // Invalid color
    _bActive = true;
    _expression = QStringLiteral("0");
    _expressionState = ExpressionState::UNKNOWN;

    _pDataMap = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer);
}

GraphData::~GraphData()
{
    _pDataMap.clear();
}

GraphData::valueAxis_t GraphData::valueAxis() const
{
    return _valueAxis;
}

void GraphData::setValueAxis(valueAxis_t axis)
{
    if (axis < VALUE_AXIS_CNT)
    {
        _valueAxis = axis;
    }
}

bool GraphData::isVisible() const
{
    return _bVisible;
}

void GraphData::setVisible(bool bVisible)
{
    _bVisible = bVisible;
}

QString GraphData::label() const
{
    return _label;
}

void GraphData::setLabel(const QString &label)
{
    /* Remove separator char (used in export) from label */
    QString cleanedLabel = QString(label).remove(Util::separatorCharacter());

    _label = cleanedLabel;
}

QColor GraphData::color() const
{
    return _color;
}

void GraphData::setColor(const QColor &color)
{
    _color = color;
}

bool GraphData::isActive() const
{
    return _bActive;
}

void GraphData::setActive(bool bActive)
{
    _bActive = bActive;
}

QString GraphData::expression() const
{
    return _expression;
}

void GraphData::setExpression(QString expression)
{
    _expression = expression;
    _expressionState = ExpressionState::UNKNOWN;
}

GraphData::ExpressionState GraphData::expressionState() const
{
    return _expressionState;
}

bool GraphData::isExpressionValid() const
{
    return (_expressionState == ExpressionState::VALID) || (_expressionState == ExpressionState::UNKNOWN);
}

void GraphData::setExpressionState(GraphData::ExpressionState status)
{
    _expressionState = status;
}

QSharedPointer<QCPGraphDataContainer> GraphData::dataMap()
{
    return _pDataMap;
}
