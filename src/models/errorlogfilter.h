#ifndef ERRORLOGFILTER_H
#define ERRORLOGFILTER_H

#include <QObject>
#include <QSortFilterProxyModel>

class ErrorLogFilter : public QSortFilterProxyModel
{

public:
    ErrorLogFilter(QObject* parent = nullptr);
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

public slots:
    void setFilterBitmask(quint32 bitmask);

private:
    quint32 _filterBitmask;

};

#endif // ERRORLOGFILTER_H
