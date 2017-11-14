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

    qint32 size() const;

    void clear();

    void addError(QString message);
    void addInfo(QString message);

signals:

public slots:

private:
    void addItem(ErrorLog::LogCategory cat, QString msg);

    QList<ErrorLog> _logList;
};

#endif // ERRORLOGMODEL_H
