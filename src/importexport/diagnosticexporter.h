#ifndef DIAGNOSTICEXPORTER_H
#define DIAGNOSTICEXPORTER_H

#include <QObject>
#include <QTextStream>

/* forward declaration */
class DiagnosticModel;

class DiagnosticExporter : public QObject
{
    Q_OBJECT
public:
    explicit DiagnosticExporter(DiagnosticModel* pDiagModel, QObject *parent = nullptr);

    void exportDiagnosticsFile(QTextStream &diagStream);

signals:

private:
    DiagnosticModel* _pDiagModel;

};

#endif // DIAGNOSTICEXPORTER_H
