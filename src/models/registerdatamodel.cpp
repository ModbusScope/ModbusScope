#include "registerdatamodel.h"
#include "util.h"

#include "registerdata.h"

RegisterDataModel::RegisterDataModel(QObject *parent) : QObject(parent)
{
    _dataList.clear();
}

qint32 RegisterDataModel::size()
{
    return _dataList.size();
}

RegisterData * RegisterDataModel::registerData(qint32 idx)
{
    if (idx < _dataList.size())
    {
        return &_dataList[idx];
    }
    else
    {
        return NULL;
    }
}

void RegisterDataModel::addRegister(RegisterData rowData)
{
    /* Select color when adding through register dialog */
    if (!rowData.color().isValid())
    {
        quint32 colorIndex = _dataList.size() % Util::cColorlist.size();
        rowData.setColor(Util::cColorlist[colorIndex]);
    }

    _dataList.append(rowData);

    emit added(_dataList.size() - 1);
}

void RegisterDataModel::addRegister()
{
    RegisterData data;
    data.setActive(true);
    data.setUnsigned(false);
    data.setAddress(nextFreeAddress());
    data.setBitmask(0xFFFF);
    data.setText(QString("Register %1 (bitmask: 0x%2)").arg(data.address()).arg(data.bitmask(), 0, 16));
    data.setDivideFactor(1);
    data.setMultiplyFactor(1);
    data.setShift(0);
    data.setColor("-1"); // Invalid color

    addRegister(data);
}

void RegisterDataModel::removeRegister(qint32 idx)
{
    if (idx < _dataList.size())
    {
        _dataList.removeAt(idx);
    }

    emit removed(idx);
}

void RegisterDataModel::clear()
{
    _dataList.clear();

    emit cleared();
}

// TODO
uint RegisterDataModel::checkedRegisterCount()
{
    uint count = 0;

    for (int i = 0; i < _dataList.size(); i++)
    {
        if (_dataList[i].isActive())
        {
            count++;
        }
    }

    return count;
}

// TODO
void RegisterDataModel::registerList(QList<quint16> * pRegisterList)
{
    pRegisterList->clear();
    for (qint32 i = 0; i < _dataList.size(); i++)
    {
        pRegisterList->append(_dataList[i].address());
    }
}

/*
 *  Get sorted list of checked register addresses
 */
// TODO
void RegisterDataModel::checkedRegisterList(QList<RegisterData> * pRegisterList)
{
    pRegisterList->clear();
    for (qint32 i = 0; i < _dataList.size(); i++)
    {
        if (_dataList[i].isActive())
        {
            pRegisterList->append(_dataList[i]);
        }
    }

    // Sort by register addresscolor
    std::sort(pRegisterList->begin(), pRegisterList->end(), &RegisterData::sortRegisterDataList);
}

bool RegisterDataModel::areExclusive(quint16 * pRegister, quint16 * pBitmask)
{
    for (qint32 idx = 0; idx < (_dataList.size() - 1); idx++) // Don't need to check last entry
    {
        for (int checkIdx = (idx + 1); checkIdx < _dataList.size(); checkIdx++)
        {
            if (
                (_dataList[idx].address() == _dataList[checkIdx].address())
                && (_dataList[idx].bitmask() == _dataList[checkIdx].bitmask())
            )
            {
                *pRegister = _dataList[idx].address();
                *pBitmask = _dataList[idx].bitmask();
                return false;
            }
        }
    }

    return true;
}

quint16 RegisterDataModel::nextFreeAddress()
{
    QList<quint16> regList;
    quint16 nextAddress;

    // get register list
    this->registerList(&regList);

    // sort qList
    qSort(regList);

    if (regList.size() > 0)
    {
        nextAddress = regList.last() + 1;
    }
    else
    {
        nextAddress = 40001;
    }

    return nextAddress;
}
