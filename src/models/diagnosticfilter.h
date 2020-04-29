#ifndef DIAGNOSTICFILTER_H
#define DIAGNOSTICFILTER_H

#include <QObject>
#include <QSortFilterProxyModel>

class DiagnosticFilter : public QSortFilterProxyModel
{

public:
    DiagnosticFilter(QObject* parent = nullptr);
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

public slots:
    void setFilterBitmask(quint32 bitmask);

private:
    quint32 _filterBitmask;

};

#endif // DIAGNOSTICFILTER_H
