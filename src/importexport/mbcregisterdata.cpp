#include "mbcregisterdata.h"
#include "updateregisternewexpression.h"
#include <QtMath>

MbcRegisterData::MbcRegisterData()
{
    _registerAddress = 0;
    _type = ModbusDataType::Type::UNSIGNED_16;
    _name = QString();
    _tabIdx = 0;
    _bReadable = true;
    _decimals = 0;
}

MbcRegisterData::~MbcRegisterData()
{

}

MbcRegisterData::MbcRegisterData(quint32 registerAddress, ModbusDataType::Type type, QString name, qint32 tabIdx, bool bReadable, quint8 decimals)
{
    _registerAddress = registerAddress;
    _type = type;
    _name = name;
    _tabIdx = tabIdx;
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

    if (_type != pMbcRegdata->type())
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

quint32 MbcRegisterData::registerAddress() const
{
    return _registerAddress;
}

void MbcRegisterData::setRegisterAddress(const quint32 &registerAddress)
{
    _registerAddress = registerAddress;
}

void MbcRegisterData::setType(ModbusDataType::Type type)
{
    _type = type;
}

ModbusDataType::Type MbcRegisterData::type() const
{
    return _type;
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
    QString registerStr = UpdateRegisterNewExpression::constructRegisterString(_registerAddress, _type, 0);
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




