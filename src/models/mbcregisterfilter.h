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
    void setTextFilter(QString filterText);

private:
    QString _tab;
    QString _textFilter;

};

#endif // MBCREGISTERFILTER_H
