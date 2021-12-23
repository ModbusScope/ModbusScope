#include "graphdata.h"

#include "util.h"

GraphData::GraphData()
{
    _bVisible = true;
    _label = QString("Unknown register");
    _color = "-1"; // Invalid color
    _bActive = true;
    _expression = QStringLiteral("0");

    _pDataMap = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer);
}

GraphData::~GraphData()
{
    _pDataMap.clear();
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
}

QSharedPointer<QCPGraphDataContainer> GraphData::dataMap()
{
    return _pDataMap;
}
