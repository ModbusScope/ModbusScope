#include "diagnosticexporter.h"

#include "diagnosticmodel.h"

DiagnosticExporter::DiagnosticExporter(DiagnosticModel * pDiagModel, QObject *parent) : QObject(parent)
{
    _pDiagModel = pDiagModel;
}

void DiagnosticExporter::exportDiagnosticsFile(QTextStream& diagStream)
{
    for (qint32 idx = 0; idx < _pDiagModel->size(); idx++)
    {
        diagStream << _pDiagModel->toExportString(idx) << "\n";
    }
}
