#include "registerdata.h"

RegisterData::RegisterData()
{
    bActive = false;
    bUnsigned = false;
    scaleFactor = 1;
    reg = 0;
    text = "";
}

void RegisterData::CopyTo(RegisterData * pData)
{
    pData->setActive(this->getActive());
    pData->setRegisterAddress(this->getRegisterAddress());
    pData->setScaleFactor(this->getScaleFactor());
    pData->setText(this->getText());
    pData->setUnsigned(this->getUnsigned());
}


bool RegisterData::getUnsigned() const
{
    return bUnsigned;
}

void RegisterData::setUnsigned(bool value)
{
    bUnsigned = value;
}

double RegisterData::getScaleFactor() const
{
    return scaleFactor;
}

void RegisterData::setScaleFactor(double value)
{
    scaleFactor = value;
}

QString RegisterData::getText() const
{
    return text;
}

void RegisterData::setText(const QString &value)
{
    text = value;
}

quint16 RegisterData::getRegisterAddress() const
{
    return reg;
}

void RegisterData::setRegisterAddress(const quint16 &value)
{
    reg = value;
}

bool RegisterData::getActive() const
{
    return bActive;
}

void RegisterData::setActive(bool value)
{
    bActive = value;
}

bool RegisterData::sortRegisterDataList(const RegisterData& s1, const RegisterData& s2)
{
    return s1.getRegisterAddress() < s2.getRegisterAddress();
}

