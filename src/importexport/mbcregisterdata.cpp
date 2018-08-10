#include "mbcregisterdata.h"

MbcRegisterData::MbcRegisterData()
{

}

MbcRegisterData::~MbcRegisterData()
{

}

MbcRegisterData::MbcRegisterData(quint16 registerAddress, bool bUnsigned, QString name, qint32 tabIdx, bool bUint32)
{
    _registerAddress = registerAddress;
    _bUnsigned = bUnsigned;
    _name = name;
    _tabIdx = tabIdx;
    _bUint32 = bUint32;
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
