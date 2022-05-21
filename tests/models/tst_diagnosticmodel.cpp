
#include <QtTest/QtTest>

#include "tst_diagnosticmodel.h"

#include "diagnosticmodel.h"

void TestDiagnosticModel::init()
{
    _category = QStringLiteral("scope.comm");
}

void TestDiagnosticModel::cleanup()
{

}

void TestDiagnosticModel::addClear()
{
    DiagnosticModel diagModel;

    QCOMPARE(diagModel.size(), 0);
    QCOMPARE(diagModel.rowCount(), 0);

    diagModel.addLog(_category, Diagnostic::LOG_INFO, 0, QStringLiteral("Test"));

    QCOMPARE(diagModel.size(), 1);
    QCOMPARE(diagModel.rowCount(), 1);

    diagModel.addLog(_category, Diagnostic::LOG_INFO, 10, QStringLiteral("Test"));

    QCOMPARE(diagModel.size(), 2);
    QCOMPARE(diagModel.rowCount(), 2);

    diagModel.clear();

    QCOMPARE(diagModel.size(), 0);
    QCOMPARE(diagModel.rowCount(), 0);
}

void TestDiagnosticModel::headerData()
{
    DiagnosticModel diagModel;

    QCOMPARE(diagModel.columnCount(QModelIndex()), 1);
    QCOMPARE(diagModel.headerData(0, Qt::Horizontal, Qt::DisplayRole), QString("Messages"));
    QCOMPARE(diagModel.headerData(0, Qt::Vertical, Qt::DisplayRole), QString("Messages"));

    QCOMPARE(diagModel.headerData(1, Qt::Horizontal, Qt::DisplayRole), QVariant());
    QCOMPARE(diagModel.headerData(1, Qt::Horizontal, Qt::EditRole), QVariant());

}

void TestDiagnosticModel::data()
{
    DiagnosticModel diagModel;

    Diagnostic logErr(_category, Diagnostic::LOG_WARNING, 0, QString("Error"));
    diagModel.addLog(logErr.category(), logErr.severity(), logErr.timeOffset(), logErr.message());

    Diagnostic logInfo(_category, Diagnostic::LOG_INFO, 10, QString("Info"));
    diagModel.addLog(logInfo.category(), logInfo.severity(), logInfo.timeOffset(), logInfo.message());

    QModelIndex index = diagModel.index(0);
    QCOMPARE(diagModel.data(index), logErr.toString());

    index = diagModel.index(1);
    QCOMPARE(diagModel.data(index), logInfo.toString());

    QModelIndex indexNull;
    QCOMPARE(diagModel.data(indexNull), QVariant());
}

void TestDiagnosticModel::dataSeverity()
{
    DiagnosticModel diagModel;

    Diagnostic logErr(_category, Diagnostic::LOG_WARNING, 0, QString("Error"));
    diagModel.addLog(logErr.category(), logErr.severity(), logErr.timeOffset(), logErr.message());

    Diagnostic logInfo(_category, Diagnostic::LOG_INFO, 10, QString("Info"));
    diagModel.addLog(logInfo.category(), logInfo.severity(), logErr.timeOffset(), logInfo.message());

    QCOMPARE(diagModel.dataSeverity(0), logErr.severity());
    QCOMPARE(diagModel.dataSeverity(1), logInfo.severity());

    QCOMPARE(diagModel.dataSeverity(255), static_cast<Diagnostic::LogSeverity>(-1));
}

void TestDiagnosticModel::flags()
{
    DiagnosticModel diagModel;

    QModelIndex index = diagModel.index(0);

    QCOMPARE(diagModel.flags(index), Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

void TestDiagnosticModel::addLog()
{
    DiagnosticModel diagModel;

    QSignalSpy spy(&diagModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)));

    Diagnostic logErr(_category, Diagnostic::LOG_WARNING, 10, QString("Error"));
    diagModel.addLog(logErr.category(), logErr.severity(), logErr.timeOffset(), logErr.message());

    QCOMPARE(spy.count(), 1);

    QModelIndex changedIndex = diagModel.index(diagModel.size()-1);

    QList<QVariant> arguments = spy.takeFirst();

    QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(0)).row(), changedIndex.row());
    QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(1)).row(), changedIndex.row());
}

QTEST_GUILESS_MAIN(TestDiagnosticModel)
