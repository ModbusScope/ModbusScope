
#include <QtTest/QtTest>

#include "testerrorlogmodel.h"

#include "errorlogmodel.h"

void TestErrorLogModel::init()
{

}

void TestErrorLogModel::cleanup()
{

}

void TestErrorLogModel::addClear()
{
    ErrorLogModel * pErrorModel = new ErrorLogModel();

    QCOMPARE(pErrorModel->size(), 0);
    QCOMPARE(pErrorModel->rowCount(), 0);

    ErrorLog log(ErrorLog::LOG_INFO, QDateTime(), QString("Test"));
    pErrorModel->addItem(log);

    QCOMPARE(pErrorModel->size(), 1);
    QCOMPARE(pErrorModel->rowCount(), 1);

    pErrorModel->addItem(log);

    QCOMPARE(pErrorModel->size(), 2);
    QCOMPARE(pErrorModel->rowCount(), 2);

    pErrorModel->clear();

    QCOMPARE(pErrorModel->size(), 0);
    QCOMPARE(pErrorModel->rowCount(), 0);
}

void TestErrorLogModel::headerData()
{
    ErrorLogModel * pErrorModel = new ErrorLogModel();

    QCOMPARE(pErrorModel->columnCount(QModelIndex()), 1);
    QCOMPARE(pErrorModel->headerData(0, Qt::Horizontal, Qt::DisplayRole), QString("Messages"));
    QCOMPARE(pErrorModel->headerData(0, Qt::Vertical, Qt::DisplayRole), QString("Messages"));

    QCOMPARE(pErrorModel->headerData(1, Qt::Horizontal, Qt::DisplayRole), QVariant());
    QCOMPARE(pErrorModel->headerData(1, Qt::Horizontal, Qt::EditRole), QVariant());

}

void TestErrorLogModel::data()
{
    ErrorLogModel * pErrorModel = new ErrorLogModel();

    QDateTime now = QDateTime::currentDateTime();
    ErrorLog logErr(ErrorLog::LOG_ERROR, now, QString("Error"));
    pErrorModel->addItem(logErr);

    ErrorLog logInfo(ErrorLog::LOG_INFO, now, QString("Info"));
    pErrorModel->addItem(logInfo);

    QModelIndex index = pErrorModel->index(0);
    QCOMPARE(pErrorModel->data(index), logErr.toString());

    index = pErrorModel->index(1);
    QCOMPARE(pErrorModel->data(index), logInfo.toString());

    QModelIndex indexNull;
    QCOMPARE(pErrorModel->data(indexNull), QVariant());
}

void TestErrorLogModel::dataCategory()
{
    ErrorLogModel * pErrorModel = new ErrorLogModel();

    QDateTime now = QDateTime::currentDateTime();
    ErrorLog logErr(ErrorLog::LOG_ERROR, now, QString("Error"));
    pErrorModel->addItem(logErr);

    ErrorLog logInfo(ErrorLog::LOG_INFO, now, QString("Info"));
    pErrorModel->addItem(logInfo);

    QCOMPARE(pErrorModel->dataCategory(0), logErr.category());
    QCOMPARE(pErrorModel->dataCategory(1), logInfo.category());

    QCOMPARE(pErrorModel->dataCategory(255), static_cast<ErrorLog::LogCategory>(-1));
}

void TestErrorLogModel::flags()
{
    ErrorLogModel * pErrorModel = new ErrorLogModel();

    QModelIndex index = pErrorModel->index(0);

    QCOMPARE(pErrorModel->flags(index), Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

void TestErrorLogModel::addItem()
{
    ErrorLogModel * pErrorModel = new ErrorLogModel();

    QSignalSpy spy(pErrorModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)));

    QDateTime now = QDateTime::currentDateTime();
    ErrorLog logErr(ErrorLog::LOG_ERROR, now, QString("Error"));
    pErrorModel->addItem(logErr);

    QCOMPARE(spy.count(), 1);

    QModelIndex changedIndex = pErrorModel->index(pErrorModel->size()-1);

    QList<QVariant> arguments = spy.takeFirst(); // take the first signal

    QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(0)).row(), changedIndex.row()); // verify the first argument
    QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(1)).row(), changedIndex.row()); // verify the second argument
}

QTEST_GUILESS_MAIN(TestErrorLogModel)
