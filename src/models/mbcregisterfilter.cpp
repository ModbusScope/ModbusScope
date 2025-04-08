#include "mbcregisterfilter.h"

#include "models/mbcregistermodel.h"

const QString MbcRegisterFilter::cTabNoFilter = QString("No Filter");

MbcRegisterFilter::MbcRegisterFilter(QObject* parent) : QSortFilterProxyModel(parent)
{
    _tab = cTabNoFilter;
    _textFilter.clear();
}

bool MbcRegisterFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (source_row < sourceModel()->rowCount())
    {
        return performTabFilter(source_row, source_parent) && performTextFilter(source_row, source_parent);
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

void MbcRegisterFilter::setTextFilter(QString filterText)
{
    if (filterText.trimmed() != _textFilter)
    {
        _textFilter = filterText.trimmed();
    }

    invalidateFilter();
}

bool MbcRegisterFilter::performTabFilter(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex tabIdx = sourceModel()->index(source_row, MbcRegisterModel::cColumnTab, source_parent);
    QString tabName = tabIdx.data().toString();

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

    return bAllowed;
}

bool MbcRegisterFilter::performTextFilter(int source_row, const QModelIndex &source_parent) const
{
    bool bAllowed = true;

    QModelIndex descriptionIdx = sourceModel()->index(source_row, MbcRegisterModel::cColumnText, source_parent);
    QString description = descriptionIdx.data().toString();

    QModelIndex addressIdx = sourceModel()->index(source_row, MbcRegisterModel::cColumnAddress, source_parent);
    QString strAddress = addressIdx.data().toString();

    /* Filter on text */
    if (
        (!_textFilter.isEmpty())
        && (!description.contains(_textFilter, Qt::CaseInsensitive))
        && (!strAddress.contains(_textFilter, Qt::CaseInsensitive))
    )
    {
        bAllowed = false;
    }

    return bAllowed;
}
