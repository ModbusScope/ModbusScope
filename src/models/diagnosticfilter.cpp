#include "diagnosticfilter.h"

#include "models/diagnosticmodel.h"

DiagnosticFilter::DiagnosticFilter(QObject* parent) : QSortFilterProxyModel(parent)
{
    _filterBitmask = (1 << Diagnostic::LOG_DEBUG) | (1 << Diagnostic::LOG_INFO) | (1 << Diagnostic::LOG_WARNING);
}

bool DiagnosticFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent);
    Diagnostic::LogSeverity severity = reinterpret_cast<DiagnosticModel *>(sourceModel())->dataSeverity(static_cast<quint32>(source_row));

    if (_filterBitmask & (1 << severity))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void DiagnosticFilter::setFilterBitmask(quint32 bitmask)
{
    if (bitmask != _filterBitmask)
    {
        _filterBitmask = bitmask;
    }

    invalidateFilter();
}
