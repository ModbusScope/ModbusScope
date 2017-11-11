#include "errorlogmodel.h"
#include "QModelIndex"
#include "QAbstractItemModel"

ErrorLogModel::ErrorLogModel(QObject *parent) : QAbstractListModel(parent)
{

}


int ErrorLogModel::rowCount(const QModelIndex & /*parent*/) const
{
    return size();
}

QVariant ErrorLogModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && (role == Qt::DisplayRole))
    {
        return _errorLogList[index.row()].toString();
    }

    return QVariant();
}

QVariant ErrorLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);

    return QString("Messages");
}

Qt::ItemFlags ErrorLogModel::flags(const QModelIndex & index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

qint32 ErrorLogModel::size() const
{
    return _errorLogList.size();
}

void ErrorLogModel::addError(QString message)
{
    addItem(ErrorLog::LOG_ERROR, message);
}

void ErrorLogModel::addInfo(QString message)
{
    addItem(ErrorLog::LOG_INFO, message);
}

void ErrorLogModel::addItem(ErrorLog::LogCategory cat, QString msg)
{
    /* Call function to prepare view */
    beginInsertRows(QModelIndex(), size(), size());

    const QDateTime timestamp = QDateTime::currentDateTime();
    _errorLogList.append(ErrorLog(cat, timestamp, msg));

#ifdef DEBUG
    //qDebug() << _errorLogList.last();
#endif

    /* Call functions to trigger view update */
    endInsertRows();

    QModelIndex nIndex = index(size() - 1, 0);

    emit dataChanged(nIndex, nIndex);
}
