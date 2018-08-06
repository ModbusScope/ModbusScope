#include "mbcregisterfilter.h"
#include "mbcregistermodel.h"

const QString MbcRegisterFilter::cTabNoFilter = QString("No Filter");

MbcRegisterFilter::MbcRegisterFilter(QObject* parent) : QSortFilterProxyModel(parent)
{
    _tab = cTabNoFilter;
}

bool MbcRegisterFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex tabIdx = sourceModel()->index(source_row, MbcRegisterModel::cColumnTab, source_parent);

    QString tabName = tabIdx.data().toString();

    if (_tab == cTabNoFilter)
    {
        return true;
    }
    else if (tabName == _tab)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void MbcRegisterFilter::setTab(QString tab)
{
    if (tab != _tab)
    {
        _tab = tab;
    }

    invalidateFilter();
}
