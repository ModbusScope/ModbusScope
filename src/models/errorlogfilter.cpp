#include "errorlogfilter.h"
#include "errorlogmodel.h"

ErrorLogFilter::ErrorLogFilter(QObject* parent) : QSortFilterProxyModel(parent)
{
    _filterBitmask = (1 << ErrorLog::LOG_INFO) | (1 << ErrorLog::LOG_ERROR);
}

bool ErrorLogFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent);
    ErrorLog::LogCategory category = reinterpret_cast<ErrorLogModel *>(sourceModel())->dataCategory(static_cast<quint32>(source_row));

    if (_filterBitmask & (1 << category))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void ErrorLogFilter::setFilterBitmask(quint32 bitmask)
{
    if (bitmask != _filterBitmask)
    {
        _filterBitmask = bitmask;
    }

    invalidateFilter();
}
