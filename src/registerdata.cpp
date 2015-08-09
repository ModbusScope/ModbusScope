#include "registerdata.h"

RegisterData::RegisterData()
{
    _bActive = false;
    _bUnsigned = false;
    _scaleFactor = 1;
    _reg = 0;
    _bitmask = 0xFFFF;
    _text = "";
    _color = QColor("-1");
    _shift = 0;
}

void RegisterData::CopyTo(RegisterData * pData)
{
    pData->setActive(this->isActive());
    pData->setColor(this->color());
    pData->setRegisterAddress(this->registerAddress());
    pData->setBitmask(this->bitmask());
    pData->setScaleFactor(this->scaleFactor());
    pData->setText(this->text());
    pData->setUnsigned(this->isUnsigned());
    pData->setShift(this->shift());
}


bool RegisterData::isUnsigned() const
{
    return _bUnsigned;
}

void RegisterData::setUnsigned(bool value)
{
    _bUnsigned = value;
}

double RegisterData::scaleFactor() const
{
    return _scaleFactor;
}

void RegisterData::setScaleFactor(double value)
{
    _scaleFactor = value;
}

QString RegisterData::text() const
{
    return _text;
}

void RegisterData::setText(const QString &value)
{
    _text = value;
}

QColor RegisterData::color() const
{
    return _color;
}

void RegisterData::setColor(const QColor &value)
{
    _color = value;
}

qint32 RegisterData::shift() const
{
    return _shift;
}

void RegisterData::setShift(const qint32 &value)
{
    _shift = value;
}

quint16 RegisterData::registerAddress() const
{
    return _reg;
}

void RegisterData::setRegisterAddress(const quint16 &value)
{
    _reg = value;
}

quint16 RegisterData::bitmask() const
{
    return _bitmask;
}

void RegisterData::setBitmask(const quint16 &value)
{
    _bitmask = value;
}

bool RegisterData::isActive() const
{
    return _bActive;
}

void RegisterData::setActive(bool value)
{
    _bActive = value;
}

bool RegisterData::sortRegisterDataList(const RegisterData& s1, const RegisterData& s2)
{
    return s1.registerAddress() < s2.registerAddress();
}

