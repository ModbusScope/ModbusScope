#ifndef ERRORLOGMODEL_H
#define ERRORLOGMODEL_H

#include <QObject>
#include "QAbstractListModel"
#include "errorlog.h"


class ErrorLogModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ErrorLogModel(QObject *parent = nullptr);

    static ErrorLogModel& Logger();

    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    ErrorLog::LogSeverity dataSeverity(quint32 index) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;

    qint32 size() const;

    void clear();

    void addItem(ErrorLog log);

signals:

public slots:

private:

    QList<ErrorLog> _logList;
};

inline ErrorLogModel& ErrorLogModel::Logger()
{
    static ErrorLogModel errorLogModel;
    return errorLogModel;
}


#endif // ERRORLOGMODEL_H
