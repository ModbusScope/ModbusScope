
#include <QtTest/QtTest>

#include "testerrorlogmodel.h"

#include "errorlogmodel.h"

void TestErrorLogModel::init()
{
    ErrorLogModel::Logger().clear();
}

void TestErrorLogModel::cleanup()
{

}

void TestErrorLogModel::addClear()
{
    ErrorLogModel * pErrorModel = &ErrorLogModel::Logger();

    QCOMPARE(pErrorModel->size(), 0);
    QCOMPARE(pErrorModel->rowCount(), 0);

    ErrorLog log(ErrorLog::LOG_COMMUNICATION, ErrorLog::LOG_INFO, QDateTime(), QString("Test"));
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

void TestErrorLogModel::headerData()
{
    ErrorLogModel * pErrorModel = &ErrorLogModel::Logger();

    QCOMPARE(pErrorModel->columnCount(QModelIndex()), 1);
    QCOMPARE(pErrorModel->headerData(0, Qt::Horizontal, Qt::DisplayRole), QString("Messages"));
    QCOMPARE(pErrorModel->headerData(0, Qt::Vertical, Qt::DisplayRole), QString("Messages"));

    QCOMPARE(pErrorModel->headerData(1, Qt::Horizontal, Qt::DisplayRole), QVariant());
    QCOMPARE(pErrorModel->headerData(1, Qt::Horizontal, Qt::EditRole), QVariant());

}

void TestErrorLogModel::data()
{
    ErrorLogModel * pErrorModel = &ErrorLogModel::Logger();

    QDateTime now = QDateTime::currentDateTime();
    ErrorLog logErr(ErrorLog::LOG_COMMUNICATION, ErrorLog::LOG_ERROR, now, QString("Error"));
    pErrorModel->addLog(logErr);

    ErrorLog logInfo(ErrorLog::LOG_COMMUNICATION, ErrorLog::LOG_INFO, now, QString("Info"));
    pErrorModel->addLog(logInfo);

    QModelIndex index = pErrorModel->index(0);
    QCOMPARE(pErrorModel->data(index), logErr.toString());

    index = pErrorModel->index(1);
    QCOMPARE(pErrorModel->data(index), logInfo.toString());

    QModelIndex indexNull;
    QCOMPARE(pErrorModel->data(indexNull), QVariant());
}

void TestErrorLogModel::dataSeverity()
{
    ErrorLogModel * pErrorModel = &ErrorLogModel::Logger();

    QDateTime now = QDateTime::currentDateTime();
    ErrorLog logErr(ErrorLog::LOG_COMMUNICATION, ErrorLog::LOG_ERROR, now, QString("Error"));
    pErrorModel->addLog(logErr);

    ErrorLog logInfo(ErrorLog::LOG_COMMUNICATION, ErrorLog::LOG_INFO, now, QString("Info"));
    pErrorModel->addLog(logInfo);

    QCOMPARE(pErrorModel->dataSeverity(0), logErr.severity());
    QCOMPARE(pErrorModel->dataSeverity(1), logInfo.severity());

    QCOMPARE(pErrorModel->dataSeverity(255), static_cast<ErrorLog::LogSeverity>(-1));
}

void TestErrorLogModel::flags()
{
    ErrorLogModel * pErrorModel = &ErrorLogModel::Logger();

    QModelIndex index = pErrorModel->index(0);

    QCOMPARE(pErrorModel->flags(index), Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

void TestErrorLogModel::addLog_1()
{
    ErrorLogModel * pErrorModel = &ErrorLogModel::Logger();

    QSignalSpy spy(pErrorModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)));

    QDateTime now = QDateTime::currentDateTime();
    ErrorLog logErr(ErrorLog::LOG_COMMUNICATION, ErrorLog::LOG_ERROR, now, QString("Error"));
    pErrorModel->addLog(logErr);

    QCOMPARE(spy.count(), 1);

    QModelIndex changedIndex = pErrorModel->index(pErrorModel->size()-1);

    QList<QVariant> arguments = spy.takeFirst(); // take the first signal

    QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(0)).row(), changedIndex.row()); // verify the first argument
    QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(1)).row(), changedIndex.row()); // verify the second argument
}

void TestErrorLogModel::addLog_2()
{
    ErrorLogModel * pErrorModel = &ErrorLogModel::Logger();

    QSignalSpy spy(pErrorModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)));

    QDateTime now = QDateTime::currentDateTime();
    pErrorModel->addCommunicationLog(ErrorLog::LOG_ERROR, QString("Error"));

    QCOMPARE(spy.count(), 1);

    QModelIndex changedIndex = pErrorModel->index(pErrorModel->size()-1);

    QList<QVariant> arguments = spy.takeFirst(); // take the first signal

    QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(0)).row(), changedIndex.row()); // verify the first argument
    QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(1)).row(), changedIndex.row()); // verify the second argument
}

void TestErrorLogModel::addCommunicationLog()
{
    ErrorLogModel * pErrorModel = &ErrorLogModel::Logger();

    QCOMPARE(pErrorModel->size(), 0);
    QCOMPARE(pErrorModel->rowCount(), 0);

    pErrorModel->addCommunicationLog(ErrorLog::LOG_ERROR, QString("Error"));

    QCOMPARE(pErrorModel->size(), 1);
    QCOMPARE(pErrorModel->rowCount(), 1);
}


QTEST_GUILESS_MAIN(TestErrorLogModel)
