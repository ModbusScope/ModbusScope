
#ifndef TST_DIAGNOSTICMODEL_H
#define TST_DIAGNOSTICMODEL_H

#include <QObject>

class TestDiagnosticModel : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void addClear();
    void clearEmpty();
    void headerData();
    void data();
    void dataSeverity();
    void flags();

    void addLog();
    void addLogLowerSeverity();
    void addLogSameSeverity();

private:
    QString _category;
};

#endif // TST_DIAGNOSTICMODEL_H
