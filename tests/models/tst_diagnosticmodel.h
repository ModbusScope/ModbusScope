
#ifndef TEST_DIAGNOSTICMODEL_H__
#define TEST_DIAGNOSTICMODEL_H__

#include <QObject>

class TestDiagnosticModel: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void addClear();
    void headerData();
    void data();
    void dataSeverity();
    void flags();

    void addLog();

private:

        QString _category;

};

#endif /* TEST_DIAGNOSTICMODEL_H__ */
