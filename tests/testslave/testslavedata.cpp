#include "testslavedata.h"
#include <QDebug>

TestSlaveData::TestSlaveData(quint32 registerCount) : QObject(nullptr)
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

uint TestSlaveData::size()
{
    if (_registerList.size() < 0)
    {
        return 0;
    }

    return static_cast<uint>(_registerList.size());
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
        if (registerAddress < _registerList.size())
        {
            if (_registerList[registerAddress].bState != bState)
            {
                _registerList[registerAddress].bState = bState;

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
    if (registerAddress < _registerList.size())
    {
        if (_registerList[registerAddress].value != value)
        {
            _registerList[registerAddress].value = value;

            emit dataChanged();
        }
    }
}

bool TestSlaveData::registerState(uint registerAddress)
{
    if (registerAddress < _registerList.size())
    {
        return _registerList[registerAddress].bState;
    }

    return false;
}

quint16 TestSlaveData::registerValue(uint registerAddress)
{
    if (registerAddress < _registerList.size())
    {
        return _registerList[registerAddress].value;
    }

    return 0;
}

void TestSlaveData::incrementAllEnabledRegisters()
{
    for(uint idx = 0u; idx < _registerList.size(); idx++)
    {
        if (_registerList[idx].bState)
        {
            _registerList[idx].value++;
        }
    }

    emit dataChanged();
}
