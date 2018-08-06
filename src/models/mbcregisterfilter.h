#ifndef MBCREGISTERFILTER_H
#define MBCREGISTERFILTER_H

#include <QObject>
#include <QSortFilterProxyModel>

class MbcRegisterFilter : public QSortFilterProxyModel
{

public:
    MbcRegisterFilter(QObject* parent = nullptr);
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

    static const QString cTabNoFilter;

public slots:
    void setTab(QString tab);

private:
    QString _tab;

};

#endif // MBCREGISTERFILTER_H
