#include "diagnosticfilter.h"

#include "models/diagnosticmodel.h"

DiagnosticFilter::DiagnosticFilter(QObject* parent) : QSortFilterProxyModel(parent)
{
    _filterBitmask = (1 << Diagnostic::LOG_DEBUG) | (1 << Diagnostic::LOG_INFO) | (1 << Diagnostic::LOG_WARNING);
}

bool DiagnosticFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_UNUSED(source_parent);
    DiagnosticModel* pModel = qobject_cast<DiagnosticModel*>(sourceModel());
    if (pModel == nullptr)
    {
        return false;
    }

    Diagnostic::LogSeverity severity = pModel->dataSeverity(static_cast<quint32>(source_row));

    return (_filterBitmask & (1 << severity)) != 0;
}

void DiagnosticFilter::setFilterBitmask(quint32 bitmask)
{
    if (bitmask != _filterBitmask)
    {
        _filterBitmask = bitmask;
    }

    invalidateFilter();
}
