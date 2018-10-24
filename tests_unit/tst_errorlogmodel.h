#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QSignalSpy>

#include "src/models/errorlogmodel.h"

using namespace testing;

TEST(ErrorLogModel, addClear)
{
    ErrorLogModel * pErrorModel = new ErrorLogModel();

    EXPECT_EQ(pErrorModel->size(), 0);
    EXPECT_EQ(pErrorModel->rowCount(), 0);

    ErrorLog log(ErrorLog::LOG_INFO, QDateTime(), QString("Test"));
    pErrorModel->addItem(log);

    EXPECT_EQ(pErrorModel->size(), 1);
    EXPECT_EQ(pErrorModel->rowCount(), 1);

    pErrorModel->addItem(log);

    EXPECT_EQ(pErrorModel->size(), 2);
    EXPECT_EQ(pErrorModel->rowCount(), 2);

    pErrorModel->clear();

    EXPECT_EQ(pErrorModel->size(), 0);
    EXPECT_EQ(pErrorModel->rowCount(), 0);
}

TEST(ErrorLogModel, headerData)
{
    ErrorLogModel * pErrorModel = new ErrorLogModel();

    EXPECT_EQ(pErrorModel->columnCount(QModelIndex()), 1);
    EXPECT_EQ(pErrorModel->headerData(0, Qt::Horizontal, Qt::DisplayRole), QString("Messages"));
    EXPECT_EQ(pErrorModel->headerData(0, Qt::Vertical, Qt::DisplayRole), QString("Messages"));

    EXPECT_EQ(pErrorModel->headerData(1, Qt::Horizontal, Qt::DisplayRole), QVariant());
    EXPECT_EQ(pErrorModel->headerData(1, Qt::Horizontal, Qt::EditRole), QVariant());

}

TEST(ErrorLogModel, data)
{
    ErrorLogModel * pErrorModel = new ErrorLogModel();

    QDateTime now = QDateTime::currentDateTime();
    ErrorLog logErr(ErrorLog::LOG_ERROR, now, QString("Error"));
    pErrorModel->addItem(logErr);

    ErrorLog logInfo(ErrorLog::LOG_INFO, now, QString("Info"));
    pErrorModel->addItem(logInfo);

    QModelIndex index = pErrorModel->index(0);
    EXPECT_EQ(pErrorModel->data(index), logErr.toString());

    index = pErrorModel->index(1);
    EXPECT_EQ(pErrorModel->data(index), logInfo.toString());

    QModelIndex indexNull;
    EXPECT_EQ(pErrorModel->data(indexNull), QVariant());
}

TEST(ErrorLogModel, flags)
{
    ErrorLogModel * pErrorModel = new ErrorLogModel();

    QModelIndex index = pErrorModel->index(0);

    EXPECT_EQ(pErrorModel->flags(index), Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

TEST(ErrorLogModel, addItem)
{
    ErrorLogModel * pErrorModel = new ErrorLogModel();

    QSignalSpy spy(pErrorModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)));

    QDateTime now = QDateTime::currentDateTime();
    ErrorLog logErr(ErrorLog::LOG_ERROR, now, QString("Error"));
    pErrorModel->addItem(logErr);

    EXPECT_EQ(spy.count(), 1);

    QModelIndex changedIndex = pErrorModel->index(pErrorModel->size()-1);

    QList<QVariant> arguments = spy.takeFirst(); // take the first signal

    EXPECT_EQ(qvariant_cast<QModelIndex>(arguments.at(0)).row(), changedIndex.row()); // verify the first argument
    EXPECT_EQ(qvariant_cast<QModelIndex>(arguments.at(1)).row(), changedIndex.row()); // verify the second argument
}

