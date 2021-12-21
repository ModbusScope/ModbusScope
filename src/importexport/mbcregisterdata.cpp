#include "mbcregisterdata.h"
#include "updateregisternewexpression.h"
#include <QtMath>

MbcRegisterData::MbcRegisterData()
{
    _registerAddress = 0;
    _bUnsigned = true;
    _name = QString();
    _tabIdx = 0;
    _bUint32 = false;
    _bReadable = true;
    _decimals = 0;
}

MbcRegisterData::~MbcRegisterData()
{

}

MbcRegisterData::MbcRegisterData(quint16 registerAddress, bool bUnsigned, QString name, qint32 tabIdx, bool bUint32, bool bReadable, quint8 decimals)
{
    _registerAddress = registerAddress;
    _bUnsigned = bUnsigned;
    _name = name;
    _tabIdx = tabIdx;
    _bUint32 = bUint32;
    _bReadable = bReadable;
    _decimals = decimals;
}

bool MbcRegisterData::compare(MbcRegisterData* pMbcRegdata)
{
    bool bRet = true;

    if (_registerAddress != pMbcRegdata->registerAddress())
    {
        bRet = false;
    }

    if (_bUnsigned != pMbcRegdata->isUnsigned())
    {
        bRet = false;
    }

    if (_tabIdx != pMbcRegdata->tabIdx())
    {
        bRet = false;
    }

    if (_name != pMbcRegdata->name())
    {
        bRet = false;
    }

    if (_bUint32 != pMbcRegdata->is32Bit())
    {
        bRet = false;
    }

    if (_bReadable != pMbcRegdata->isReadable())
    {
        bRet = false;
    }

    if (_decimals != pMbcRegdata->decimals())
    {
        bRet = false;
    }

    return bRet;
}

quint16 MbcRegisterData::registerAddress() const
{
    return _registerAddress;
}

void MbcRegisterData::setRegisterAddress(const quint16 &registerAddress)
{
    _registerAddress = registerAddress;
}

bool MbcRegisterData::isUnsigned() const
{
    return _bUnsigned;
}

void MbcRegisterData::setUnsigned(bool bUnsigned)
{
    _bUnsigned = bUnsigned;
}

QString MbcRegisterData::name() const
{
    return _name;
}

void MbcRegisterData::setName(const QString &name)
{
    _name = name;
}

qint32 MbcRegisterData::tabIdx() const
{
    return _tabIdx;
}

void MbcRegisterData::setTabIdx(const qint32 &tabIdx)
{
    _tabIdx = tabIdx;
}

bool MbcRegisterData::is32Bit() const
{
    return _bUint32;
}

void MbcRegisterData::set32Bit(bool bUint32)
{
    _bUint32 = bUint32;
}

bool MbcRegisterData::isReadable() const
{
    return _bReadable;
}

void MbcRegisterData::setReadable(bool isReadable)
{
    _bReadable = isReadable;
}

quint8 MbcRegisterData::decimals() const
{
    return _decimals;
}

void MbcRegisterData::setDecimals(const quint8 &decimals)
{
    _decimals = decimals;
}

QString MbcRegisterData::toExpression()
{
    QString expression;

    QString registerStr = UpdateRegisterNewExpression::constructRegisterString(_registerAddress, _bUint32, _bUnsigned, 0);
    if (_decimals != 0)
    {
        expression = QString("%1/%2").arg(registerStr)
                                     .arg(static_cast<double>(qPow(10, _decimals)));
    }
    else
    {
        expression = registerStr;
    }

    return expression;
}




