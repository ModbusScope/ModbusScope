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
    _bitmask = 0xFFFFFFFF;
    _divideFactor = 1;
    _multiplyFactor = 1;
    _shift = 0;
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

double GraphData::multiplyFactor() const
{
    return _multiplyFactor;
}

void GraphData::setMultiplyFactor(double multiplyFactor)
{
    _multiplyFactor = multiplyFactor;
}

double GraphData::divideFactor() const
{
    return _divideFactor;
}

void GraphData::setDivideFactor(double divideFactor)
{
    _divideFactor = divideFactor;
}

quint16 GraphData::registerAddress() const
{
    return _registerAddress;
}

void GraphData::setRegisterAddress(const quint16 &registerAddress)
{
    _registerAddress = registerAddress;
}

quint32 GraphData::bitmask() const
{
    return _bitmask;
}

void GraphData::setBitmask(const quint32 &bitmask)
{
    _bitmask = bitmask;
}

qint32 GraphData::shift() const
{
    return _shift;
}

void GraphData::setShift(const qint32 &shift)
{
    _shift = shift;
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
