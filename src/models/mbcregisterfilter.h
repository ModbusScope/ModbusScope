#ifndef MBCREGISTERFILTER_H
#define MBCREGISTERFILTER_H

#include <QSortFilterProxyModel>

class MbcRegisterFilter : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    MbcRegisterFilter(QObject* parent = nullptr);
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    static const QString cTabNoFilter;

public slots:
    void setTab(QString tab);
    void setTextFilter(QString filterText);

private:

    bool performTabFilter(int source_row, const QModelIndex &source_parent) const;
    bool performTextFilter(int source_row, const QModelIndex &source_parent) const;

    QString _tab;
    QString _textFilter;

};

#endif // MBCREGISTERFILTER_H
