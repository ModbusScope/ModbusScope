#include "registermodel.h"

#include "QDebug"
#include <QMessageBox>

RegisterModel::RegisterModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    dataList.clear();
}

int RegisterModel::rowCount(const QModelIndex & /*parent*/) const
{
    return dataList.size();
}

int RegisterModel::columnCount(const QModelIndex & /*parent*/) const
{
    /*
    * bActive
    * Register
    * Text
    * */
    return 3; // Number of visible members of struct
}

QVariant RegisterModel::data(const QModelIndex &index, int role) const
{

    switch (index.column())
    {
    case 0:
        if (role == Qt::CheckStateRole)
        {
            if (dataList[index.row()].bActive)
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
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return dataList[index.row()].reg;
        }
        break;
    case 2:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return dataList[index.row()].text;
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
                return QString("Active");
            case 1:
                return QString("Register");
            case 2:
                return QString("Text");
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
                dataList[index.row()].bActive = true;
            }
            else
            {
                dataList[index.row()].bActive = false;
            }
        }
        break;
    case 1:
        if (role == Qt::EditRole)
        {
            const quint16 newAddr = value.toInt();
            if (
                (dataList[index.row()].reg != newAddr)
                && (IsAlreadyPresent(newAddr))
                )
            {
                bRet = false;
            }
            else
            {
                dataList[index.row()].reg = newAddr;
            }
        }
        break;
    case 2:
        if (role == Qt::EditRole)
        {
            dataList[index.row()].text = value.toString();
        }
        break;
    default:
        break;

    }

    return bRet;
}

Qt::ItemFlags RegisterModel::flags(const QModelIndex & index) const
{
    if (index.column() == 0)
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
        if (row < dataList.size())
        {
            dataList.removeAt(row);
        }
    }
    endRemoveRows();

    return true;
}

bool RegisterModel::insertRows (int row, int count, const QModelIndex &parent)
{

    if (
        (count != 1)
        || (row != dataList.size())
        )
    {
        qDebug() << "RegisterModel: Not supported";
    }

    Q_UNUSED(row);
    Q_UNUSED(count);
    beginInsertRows(parent, dataList.size(), dataList.size());

    RegisterData data;
    data.bActive = false;
    data.reg = getNextFreeAddress();
    data.text = QString("Register %1").arg(data.reg);
    dataList.append(data);

    endInsertRows();

    return true;
}

uint RegisterModel::checkedRegisterCount()
{
    uint count = 0;

    for (int i = 0; i < dataList.size(); i++)
    {
        if (dataList[i].bActive)
        {
            count++;
        }
    }

    return count;
}

void RegisterModel::getRegisterList(QList<quint16> * pRegisterList)
{
    pRegisterList->clear();
    for (int i = 0; i < dataList.size(); i++)
    {
        pRegisterList->append(dataList[i].reg);
    }
}

void RegisterModel::getCheckedRegisterList(QList<quint16> * pRegisterList)
{
    pRegisterList->clear();
    for (int i = 0; i < dataList.size(); i++)
    {
        if (dataList[i].bActive)
        {
            pRegisterList->append(dataList[i].reg);
        }
    }
}

void RegisterModel::getCheckedRegisterTextList(QList<QString> * pRegisterTextList)
{
    pRegisterTextList->clear();
    for (int i = 0; i < dataList.size(); i++)
    {
        if (dataList[i].bActive)
        {
            // Set default text with register address when there is no text
            if (dataList[i].text.isEmpty())
            {
                dataList[i].text = QString("Register %1").arg(dataList[i].reg);
            }

            pRegisterTextList->append(dataList[i].text);
        }
    }
}

void RegisterModel::clear()
{
    if (dataList.size() != 0)
    {
        removeRows(0, dataList.size(), QModelIndex());
    }
}

void RegisterModel::appendRow(RegisterData rowData)
{
    insertRows(dataList.size(), 1, QModelIndex());

    dataList[dataList.size() - 1].bActive = rowData.bActive;
    dataList[dataList.size() - 1].reg = rowData.reg;
    dataList[dataList.size() - 1].text = rowData.text;
}

bool RegisterModel::IsAlreadyPresent(quint16 newReg)
{
    bool bFound = false;

    for (int i = 0; i < dataList.size(); i++)
    {
        if (dataList[i].reg == newReg)
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
