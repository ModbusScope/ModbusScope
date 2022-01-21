#include "testslavedata.h"

TestSlaveData::TestSlaveData(quint32 offset, quint32 registerCount)
    : QObject(nullptr),
      _offset(offset)
{
    _registerList.clear();
    for(quint32 idx = 0u; idx < registerCount; idx++)
    {
        _registerList.append({false, 0});
    }
}

TestSlaveData::~TestSlaveData()
{

}

bool TestSlaveData::IsValidAddress(quint32 startAddress, quint32 valueCount)
{
    bool isValid = false;
    if (startAddress >= _offset)
    {
        quint32 regIdx = startAddress - _offset;
        if (regIdx + valueCount <= static_cast<uint>(_registerList.size()))
        {
            isValid = true;
        }
    }

    return isValid;
}

void TestSlaveData::setRegisterState(uint registerAddress, bool bState)
{
    setRegisterState(QList<uint>() << registerAddress, bState);
}

void TestSlaveData::setRegisterState(QList<uint> registerAddressList, bool bState)
{
    bool bChanged = false;

    Q_FOREACH(uint registerAddress, registerAddressList)
    {
        Q_ASSERT(registerAddress >= _offset);

        uint regIndex = registerAddress - _offset;
        if (static_cast<int>(regIndex) < _registerList.size())
        {
            if (_registerList[regIndex].bState != bState)
            {
                _registerList[regIndex].bState = bState;

                bChanged = true;
            }
        }
    }

    if (bChanged)
    {
        emit dataChanged();
    }
}

void TestSlaveData::setRegisterValue(uint registerAddress, quint16 value)
{
    Q_ASSERT(registerAddress >= _offset);
    uint regIndex = registerAddress - _offset;

    if (static_cast<int>(regIndex) < _registerList.size())
    {
        if (_registerList[regIndex].value != value)
        {
            _registerList[regIndex].value = value;

            emit dataChanged();
        }
    }
}

bool TestSlaveData::registerState(uint registerAddress)
{
    Q_ASSERT(registerAddress >= _offset);
    uint regIndex = registerAddress - _offset;

    if (static_cast<int>(regIndex) < _registerList.size())
    {
        return _registerList[regIndex].bState;
    }

    return false;
}

quint16 TestSlaveData::registerValue(uint registerAddress)
{
    Q_ASSERT(registerAddress >= _offset);
    uint regIndex = registerAddress - _offset;

    if (static_cast<int>(regIndex) < _registerList.size())
    {
        return _registerList[regIndex].value;
    }

    return 0;
}

void TestSlaveData::incrementAllEnabledRegisters()
{
    for(int idx = 0u; idx < _registerList.size(); idx++)
    {
        if (_registerList[idx].bState)
        {
            _registerList[idx].value++;
        }
    }

    emit dataChanged();
}
