#include "mbcregisterfilter.h"
#include "mbcregistermodel.h"

const QString MbcRegisterFilter::cTabNoFilter = QString("No Filter");

MbcRegisterFilter::MbcRegisterFilter(QObject* parent) : QSortFilterProxyModel(parent)
{
    _tab = cTabNoFilter;
    _textFilter.clear();
}

bool MbcRegisterFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex tabIdx = sourceModel()->index(source_row, MbcRegisterModel::cColumnTab, source_parent);
    QModelIndex descriptionIdx = sourceModel()->index(source_row, MbcRegisterModel::cColumnText, source_parent);

    QString tabName = tabIdx.data().toString();
    QString description = descriptionIdx.data().toString();

    bool bAllowed = true;

    /* Filter on tab */
    if (_tab == cTabNoFilter)
    {
        bAllowed = true;
    }
    else if (tabName == _tab)
    {
        bAllowed = true;
    }
    else
    {
        bAllowed = false;
    }

    /* Filter on text */
    if (
        bAllowed
        && (!_textFilter.isEmpty())
        && (!description.contains(_textFilter, Qt::CaseInsensitive))
    )
    {
        bAllowed = false;
    }

    return bAllowed;
}

void MbcRegisterFilter::setTab(QString tab)
{
    if (tab != _tab)
    {
        _tab = tab;
    }

    invalidateFilter();
}

void MbcRegisterFilter::setTextFilter(QString filterText)
{
    if (filterText.trimmed() != _textFilter)
    {
        _textFilter = filterText.trimmed();
    }

    invalidateFilter();
}
