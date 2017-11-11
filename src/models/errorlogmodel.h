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

    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
#if 0
    bool setData(const QModelIndex & index, const QVariant & value, int role);
    bool removeRows (int row, int count, const QModelIndex &parent);
    bool insertRows (int row, int count, const QModelIndex &parent);
#endif

    qint32 size() const;

    void addError(QString message);
    void addInfo(QString message);

signals:

public slots:

private:
    void addItem(ErrorLog::LogCategory cat, QString msg);

    QList<ErrorLog> _errorLogList;
};

#endif // ERRORLOGMODEL_H
