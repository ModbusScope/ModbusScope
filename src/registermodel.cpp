#include "registermodel.h"
#include "util.h"
#include "QDebug"
#include <QMessageBox>

RegisterModel::RegisterModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    _dataList.clear();
}

int RegisterModel::rowCount(const QModelIndex & /*parent*/) const
{
    return _dataList.size();
}

int RegisterModel::columnCount(const QModelIndex & /*parent*/) const
{
    /*
    * bActive
    * bUnsigned
    * Register
    * Text
    * Scale factor
    * */
    return 5; // Number of visible members of struct
}

QVariant RegisterModel::data(const QModelIndex &index, int role) const
{

    switch (index.column())
    {
    case 0:
        if (role == Qt::CheckStateRole)
        {
            if (_dataList[index.row()].bActive)
            {
                return Qt::Checked;
            }
            else
            {
                return Qt::Unchecked;
            }
        }
        break;
    case 1:
        if (role == Qt::CheckStateRole)
        {
            if (_dataList[index.row()].bUnsigned)
            {
                return Qt::Checked;
            }
            else
            {
                return Qt::Unchecked;
            }
        }
        break;
    case 2:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return _dataList[index.row()].reg;
        }
        break;
    case 3:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return _dataList[index.row()].text;
        }
        break;
    case 4:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return Util::formatDoubleForExport(_dataList[index.row()].scaleFactor);
        }
        break;
    default:
        return QVariant();
        break;

    }

    return QVariant();
}

QVariant RegisterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
            case 0:
                return QString("");
            case 1:
                return QString("u");
            case 2:
                return QString("Register");
            case 3:
                return QString("Text");
            case 4:
                return QString("Scale");
            default:
                return QVariant();
            }
        }
        else
        {
            //Can't happen because it is hidden
        }
    }

    return QVariant();
}


bool RegisterModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    bool bRet = true;

    switch (index.column())
    {
    case 0:
        if (role == Qt::CheckStateRole)
        {
            if (value == Qt::Checked)
            {
                _dataList[index.row()].bActive = true;
            }
            else
            {
                _dataList[index.row()].bActive = false;
            }
        }
        break;
    case 1:
        if (role == Qt::CheckStateRole)
        {
            if (value == Qt::Checked)
            {
                _dataList[index.row()].bUnsigned = true;
            }
            else
            {
                _dataList[index.row()].bUnsigned = false;
            }
        }
        break;
    case 2:
        if (role == Qt::EditRole)
        {
            const quint16 newAddr = value.toInt();
            if (
                (_dataList[index.row()].reg != newAddr)
                && (IsAlreadyPresent(newAddr))
                )
            {
                bRet = false;
            }
            else
            {
                _dataList[index.row()].reg = newAddr;
            }
        }
        break;
    case 3:
        if (role == Qt::EditRole)
        {
            _dataList[index.row()].text = value.toString();
        }
        break;
    case 4:
        if (role == Qt::EditRole)
        {
            bool bSuccess = false;
            const double parseResult = QLocale::system().toDouble(value.toString(), &bSuccess);
            if (bSuccess)
            {
                _dataList[index.row()].scaleFactor = parseResult;
            }
            else
            {
                bRet = false;
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("ModbusScope data error"));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Scale factor is not a valid double. Did you use correct decimal separator character? Expecting \"%1\"").arg(QLocale::system().decimalPoint()));
                msgBox.exec();
                break;
            }
        }
        break;
    default:
        break;

    }

    // Notify view(s) of change
    emit dataChanged(index, index);

    return bRet;
}

Qt::ItemFlags RegisterModel::flags(const QModelIndex & index) const
{
    if (
            (index.column() == 0)
            || (index.column() == 1)
        )
    {
        // checkable
        return Qt::ItemIsSelectable |  Qt::ItemIsUserCheckable | Qt::ItemIsEnabled ;
    }
    else
    {
        return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
    }

}


bool RegisterModel::removeRows (int row, int count, const QModelIndex & parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    for (qint32 i = 0; i < count; i++)
    {
        Q_UNUSED(i);
        if (row < _dataList.size())
        {
            _dataList.removeAt(row);
        }
    }
    endRemoveRows();

    // Notify view(s) of change
    emit dataChanged(parent, parent);

    return true;
}

bool RegisterModel::insertRows (int row, int count, const QModelIndex &parent)
{

    if (
        (count != 1)
        || (row != _dataList.size())
        )
    {
        qDebug() << "RegisterModel: Not supported";
    }

    Q_UNUSED(row);
    Q_UNUSED(count);
    beginInsertRows(parent, _dataList.size(), _dataList.size());

    RegisterData data;
    data.bActive = true;
    data.bUnsigned = false;
    data.reg = getNextFreeAddress();
    data.text = QString("Register %1").arg(data.reg);
    data.scaleFactor = 1;
    _dataList.append(data);

    endInsertRows();

    // Notify view(s) of change
    emit dataChanged(parent, parent);

    return true;
}

uint RegisterModel::checkedRegisterCount()
{
    uint count = 0;

    for (int i = 0; i < _dataList.size(); i++)
    {
        if (_dataList[i].bActive)
        {
            count++;
        }
    }

    return count;
}

void RegisterModel::getRegisterList(QList<quint16> * pRegisterList)
{
    pRegisterList->clear();
    for (int i = 0; i < _dataList.size(); i++)
    {
        pRegisterList->append(_dataList[i].reg);
    }
}

/*
 *  Get sorted list of checked register addresses
 */
void RegisterModel::getCheckedRegisterList(QList<ScopeData::RegisterData> * pRegisterList)
{
    pRegisterList->clear();
    for (int i = 0; i < _dataList.size(); i++)
    {
        if (_dataList[i].bActive)
        {
            ScopeData::RegisterData tmpData;
            tmpData.bUnsigned = _dataList[i].bUnsigned;
            tmpData.reg = _dataList[i].reg;
            tmpData.scaleFactor = _dataList[i].scaleFactor;

            pRegisterList->append(tmpData);
        }
    }

    // Sort by register address
    std::sort(pRegisterList->begin(), pRegisterList->end(), &ScopeData::sortRegisterDataList);
}

/*
 *  Get list of checked register texts (sorted on register address)
 */
void RegisterModel::getCheckedRegisterTextList(QList<QString> * pRegisterTextList)
{
    pRegisterTextList->clear();

    QList<RegisterData> sortedRegisterList;

    // Get checked registers
    for (int i = 0; i < _dataList.size(); i++)
    {
        if (_dataList[i].bActive)
        {
            sortedRegisterList.append(_dataList[i]);
        }
    }

    // Sort by register address
    std::sort(sortedRegisterList.begin(), sortedRegisterList.end(), &RegisterModel::sortRegisterByAddress);

    // Create text list
    for (int i = 0; i < sortedRegisterList.size(); i++)
    {
        // Set default text with register address when there is no text
        if (sortedRegisterList[i].text.isEmpty())
        {
            sortedRegisterList[i].text = QString("Register %1").arg(sortedRegisterList[i].reg);
        }

        pRegisterTextList->append(sortedRegisterList[i].text);
    }
}

void RegisterModel::clear(const QModelIndex &parent)
{
    if (_dataList.size() != 0)
    {
        removeRows(0, _dataList.size(), QModelIndex());
    }

    // Notify view(s) of change
    emit dataChanged(parent, parent);
}

void RegisterModel::appendRow(RegisterData rowData, const QModelIndex &parent)
{
    insertRows(_dataList.size(), 1, QModelIndex());

    _dataList[_dataList.size() - 1].bActive = rowData.bActive;
    _dataList[_dataList.size() - 1].bUnsigned = rowData.bUnsigned;
    _dataList[_dataList.size() - 1].reg = rowData.reg;
    _dataList[_dataList.size() - 1].text = rowData.text;
    _dataList[_dataList.size() - 1].scaleFactor = rowData.scaleFactor;

    // Notify view(s) of change
    emit dataChanged(parent, parent);
}

bool RegisterModel::sortRegisterByAddress(const RegisterData& s1, const RegisterData& s2)
{
    return s1.reg < s2.reg;
}

bool RegisterModel::IsAlreadyPresent(quint16 newReg)
{
    bool bFound = false;

    for (int i = 0; i < _dataList.size(); i++)
    {
        if (_dataList[i].reg == newReg)
        {
            bFound = true;
            break;
        }
    }

    if (bFound)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Duplicate register!"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("The register is already present in the list."));
        msgBox.exec();
    }

    return bFound;
}

quint16 RegisterModel::getNextFreeAddress()
{
    QList<quint16> registerList;
    quint16 nextFreeAddress;

    // get register list
    getRegisterList(&registerList);

    // sort qList
    qSort(registerList);

    if (registerList.size() > 0)
    {
        nextFreeAddress = registerList.last() + 1;
    }
    else
    {
        nextFreeAddress = 40001;
    }

    return nextFreeAddress;
}
