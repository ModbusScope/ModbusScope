
#include <QtTest/QtTest>

#include "testdiagnosticmodel.h"

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

    diagModel.addLog(_category, Diagnostic::LOG_INFO, QStringLiteral("Test"));

    QCOMPARE(diagModel.size(), 1);
    QCOMPARE(diagModel.rowCount(), 1);

    diagModel.addLog(_category, Diagnostic::LOG_INFO, QStringLiteral("Test"));

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

    QDateTime now = QDateTime::currentDateTime();
    Diagnostic logErr(_category, Diagnostic::LOG_WARNING, now, QString("Error"));
    diagModel.addLog(logErr.category(), logErr.severity(), logErr.message());

    Diagnostic logInfo(_category, Diagnostic::LOG_INFO, now, QString("Info"));
    diagModel.addLog(logInfo.category(), logInfo.severity(), logInfo.message());

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

    QDateTime now = QDateTime::currentDateTime();
    Diagnostic logErr(_category, Diagnostic::LOG_WARNING, now, QString("Error"));
    diagModel.addLog(logErr.category(), logErr.severity(), logErr.message());

    Diagnostic logInfo(_category, Diagnostic::LOG_INFO, now, QString("Info"));
    diagModel.addLog(logInfo.category(), logInfo.severity(), logInfo.message());

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

    QDateTime now = QDateTime::currentDateTime();
    Diagnostic logErr(_category, Diagnostic::LOG_WARNING, now, QString("Error"));
    diagModel.addLog(logErr.category(), logErr.severity(), logErr.message());

    QCOMPARE(spy.count(), 1);

    QModelIndex changedIndex = diagModel.index(diagModel.size()-1);

    QList<QVariant> arguments = spy.takeFirst(); // take the first signal

    QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(0)).row(), changedIndex.row()); // verify the first argument
    QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(1)).row(), changedIndex.row()); // verify the second argument
}

QTEST_GUILESS_MAIN(TestDiagnosticModel)
