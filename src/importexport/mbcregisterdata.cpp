#include "mbcregisterdata.h"

MbcRegisterData::MbcRegisterData()
{

}

MbcRegisterData::~MbcRegisterData()
{

}

MbcRegisterData::MbcRegisterData(quint16 registerAddress, bool bUnsigned, QString name, qint32 tabIdx, bool bUint32, bool bReadable)
{
    _registerAddress = registerAddress;
    _bUnsigned = bUnsigned;
    _name = name;
    _tabIdx = tabIdx;
    _bUint32 = bUint32;
    _bReadable = bReadable;
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
