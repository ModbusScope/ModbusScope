#include "graphdata.h"

#include "util.h"

GraphData::GraphData()
{
    _bVisible = true;
    _label = QString("Unknown register");
    _color = "-1"; // Invalid color
    _bActive = true;
    _bUnsigned = true;
    _bit32 = false;
    _registerAddress = 0;
    _expression = QStringLiteral("0");
    _connectionId = 0;

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

bool GraphData::isUnsigned() const
{
    return _bUnsigned;
}

void GraphData::setUnsigned(bool bUnsigned)
{
    _bUnsigned = bUnsigned;
}

bool GraphData::isBit32() const
{
    return _bit32;
}

void GraphData::setBit32(bool isBit32)
{
    _bit32 = isBit32;
}

QString GraphData::expression() const
{
    return _expression;
}

void GraphData::setExpression(QString expression)
{
    _expression = expression;
}

quint16 GraphData::registerAddress() const
{
    return _registerAddress;
}

void GraphData::setRegisterAddress(const quint16 &registerAddress)
{
    _registerAddress = registerAddress;
}

quint8 GraphData::connectionId() const
{
    return _connectionId;
}

void GraphData::setConnectionId(const quint8 &connectionId)
{
    _connectionId = connectionId;
}

QSharedPointer<QCPGraphDataContainer> GraphData::dataMap()
{
    return _pDataMap;
}
