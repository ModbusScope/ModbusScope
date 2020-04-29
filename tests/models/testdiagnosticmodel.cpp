
#include <QtTest/QtTest>

#include "testdiagnosticmodel.h"

#include "diagnosticmodel.h"

void TestDiagnosticModel::init()
{
    DiagnosticModel::Logger().clear();
}

void TestDiagnosticModel::cleanup()
{

}

void TestDiagnosticModel::addClear()
{
    DiagnosticModel * pErrorModel = &DiagnosticModel::Logger();

    QCOMPARE(pErrorModel->size(), 0);
    QCOMPARE(pErrorModel->rowCount(), 0);

    Diagnostic log(Diagnostic::LOG_COMMUNICATION, Diagnostic::LOG_INFO, QDateTime(), QString("Test"));
    pErrorModel->addLog(log);

    QCOMPARE(pErrorModel->size(), 1);
    QCOMPARE(pErrorModel->rowCount(), 1);

    pErrorModel->addLog(log);

    QCOMPARE(pErrorModel->size(), 2);
    QCOMPARE(pErrorModel->rowCount(), 2);

    pErrorModel->clear();

    QCOMPARE(pErrorModel->size(), 0);
    QCOMPARE(pErrorModel->rowCount(), 0);
}

void TestDiagnosticModel::headerData()
{
    DiagnosticModel * pErrorModel = &DiagnosticModel::Logger();

    QCOMPARE(pErrorModel->columnCount(QModelIndex()), 1);
    QCOMPARE(pErrorModel->headerData(0, Qt::Horizontal, Qt::DisplayRole), QString("Messages"));
    QCOMPARE(pErrorModel->headerData(0, Qt::Vertical, Qt::DisplayRole), QString("Messages"));

    QCOMPARE(pErrorModel->headerData(1, Qt::Horizontal, Qt::DisplayRole), QVariant());
    QCOMPARE(pErrorModel->headerData(1, Qt::Horizontal, Qt::EditRole), QVariant());

}

void TestDiagnosticModel::data()
{
    DiagnosticModel * pErrorModel = &DiagnosticModel::Logger();

    QDateTime now = QDateTime::currentDateTime();
    Diagnostic logErr(Diagnostic::LOG_COMMUNICATION, Diagnostic::LOG_ERROR, now, QString("Error"));
    pErrorModel->addLog(logErr);

    Diagnostic logInfo(Diagnostic::LOG_COMMUNICATION, Diagnostic::LOG_INFO, now, QString("Info"));
    pErrorModel->addLog(logInfo);

    QModelIndex index = pErrorModel->index(0);
    QCOMPARE(pErrorModel->data(index), logErr.toString());

    index = pErrorModel->index(1);
    QCOMPARE(pErrorModel->data(index), logInfo.toString());

    QModelIndex indexNull;
    QCOMPARE(pErrorModel->data(indexNull), QVariant());
}

void TestDiagnosticModel::dataSeverity()
{
    DiagnosticModel * pErrorModel = &DiagnosticModel::Logger();

    QDateTime now = QDateTime::currentDateTime();
    Diagnostic logErr(Diagnostic::LOG_COMMUNICATION, Diagnostic::LOG_ERROR, now, QString("Error"));
    pErrorModel->addLog(logErr);

    Diagnostic logInfo(Diagnostic::LOG_COMMUNICATION, Diagnostic::LOG_INFO, now, QString("Info"));
    pErrorModel->addLog(logInfo);

    QCOMPARE(pErrorModel->dataSeverity(0), logErr.severity());
    QCOMPARE(pErrorModel->dataSeverity(1), logInfo.severity());

    QCOMPARE(pErrorModel->dataSeverity(255), static_cast<Diagnostic::LogSeverity>(-1));
}

void TestDiagnosticModel::flags()
{
    DiagnosticModel * pErrorModel = &DiagnosticModel::Logger();

    QModelIndex index = pErrorModel->index(0);

    QCOMPARE(pErrorModel->flags(index), Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

void TestDiagnosticModel::addLog_1()
{
    DiagnosticModel * pErrorModel = &DiagnosticModel::Logger();

    QSignalSpy spy(pErrorModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)));

    QDateTime now = QDateTime::currentDateTime();
    Diagnostic logErr(Diagnostic::LOG_COMMUNICATION, Diagnostic::LOG_ERROR, now, QString("Error"));
    pErrorModel->addLog(logErr);

    QCOMPARE(spy.count(), 1);

    QModelIndex changedIndex = pErrorModel->index(pErrorModel->size()-1);

    QList<QVariant> arguments = spy.takeFirst(); // take the first signal

    QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(0)).row(), changedIndex.row()); // verify the first argument
    QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(1)).row(), changedIndex.row()); // verify the second argument
}

void TestDiagnosticModel::addLog_2()
{
    DiagnosticModel * pErrorModel = &DiagnosticModel::Logger();

    QSignalSpy spy(pErrorModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)));

    QDateTime now = QDateTime::currentDateTime();
    pErrorModel->addCommunicationLog(Diagnostic::LOG_ERROR, QString("Error"));

    QCOMPARE(spy.count(), 1);

    QModelIndex changedIndex = pErrorModel->index(pErrorModel->size()-1);

    QList<QVariant> arguments = spy.takeFirst(); // take the first signal

    QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(0)).row(), changedIndex.row()); // verify the first argument
    QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(1)).row(), changedIndex.row()); // verify the second argument
}

void TestDiagnosticModel::addCommunicationLog()
{
    DiagnosticModel * pErrorModel = &DiagnosticModel::Logger();

    QCOMPARE(pErrorModel->size(), 0);
    QCOMPARE(pErrorModel->rowCount(), 0);

    pErrorModel->addCommunicationLog(Diagnostic::LOG_ERROR, QString("Error"));

    QCOMPARE(pErrorModel->size(), 1);
    QCOMPARE(pErrorModel->rowCount(), 1);
}


QTEST_GUILESS_MAIN(TestDiagnosticModel)
