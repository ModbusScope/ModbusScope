
#include "gmock/gmock.h"
#include <gmock/gmock-matchers.h>

#include <QtTest/QtTest>

#include "testupdatenotify.h"

#include "../mocks/gmockutils.h"

#include "../mocks/mockversiondownloader.h"

#include "updatenotify.h"

using ::testing::Return;
using namespace testing;

// Configured to fixed version 2.1.0
/* TODO: Mock Util::GetVersion */

const QString TestUpdateNotify::_cVersionLowerRevision = QString("2.2.0");
const QString TestUpdateNotify::_cVersionLowerMinor = QString("2.1.0");
const QString TestUpdateNotify::_cVersionLowerMajor = QString("1.1.0");

const QString TestUpdateNotify::_cVersionHigherRevision = QString("2.2.1");
const QString TestUpdateNotify::_cVersionHigherMinor = QString("2.3.0");
const QString TestUpdateNotify::_cVersionHigherMajor = QString("3.1.0");

void TestUpdateNotify::init()
{
    // IMPORTANT: This must be called before any mock object constructors
    GMockUtils::InitGoogleMock();

    qRegisterMetaType<UpdateNotify::UpdateState>("UpdateNotify::UpdateState");

    _pVersionDownloader = new MockVersionDownloader();
}

void TestUpdateNotify::cleanup()
{
    delete _pVersionDownloader;
}

void TestUpdateNotify::triggerDownloader()
{
    UpdateNotify updateNotify(_pVersionDownloader);

    EXPECT_CALL(*_pVersionDownloader, performCheck())
        .Times(1)
        ;

    updateNotify.checkForUpdate();
}

void TestUpdateNotify::versionLowerRevision()
{
    checkServerCheck(_cVersionLowerRevision, UpdateNotify::VERSION_LATEST);
}

void TestUpdateNotify::versionLowerMinor()
{
    checkServerCheck(_cVersionLowerMinor, UpdateNotify::VERSION_LATEST);
}

void TestUpdateNotify::versionLowerMajor()
{
    checkServerCheck(_cVersionLowerMajor, UpdateNotify::VERSION_LATEST);
}

void TestUpdateNotify::versionEqual()
{
    configureServerData(QString("v1.2.3"), QString("http://google.be"));

    UpdateNotify updateNotify(_pVersionDownloader);

    QSignalSpy spyUpdateResult(&updateNotify, &UpdateNotify::updateCheckResult);

    _pVersionDownloader->versionDownloaded();

    QCOMPARE(spyUpdateResult.count(), 1);

    QList<QVariant> arguments = spyUpdateResult.takeFirst(); // take the first signal
    QCOMPARE(arguments.count(), 1);
    QCOMPARE(arguments.first().toUInt(), UpdateNotify::VERSION_LATEST);

    QCOMPARE(updateNotify.version(), QString("1.2.3"));
}

void TestUpdateNotify::versionHigherRevision()
{
    checkServerCheck(_cVersionHigherRevision, UpdateNotify::VERSION_UPDATE_AVAILABLE);
}

void TestUpdateNotify::versionHigherMinor()
{
    checkServerCheck(_cVersionHigherMinor, UpdateNotify::VERSION_UPDATE_AVAILABLE);
}

void TestUpdateNotify::versionHigherMajor()
{
    checkServerCheck(_cVersionHigherMajor, UpdateNotify::VERSION_UPDATE_AVAILABLE);
}

void TestUpdateNotify::incorrectVersion()
{
    configureServerData(QString("v1.2"), QString("http://google.be"));

    UpdateNotify updateNotify(_pVersionDownloader);

    QSignalSpy spyUpdateResult(&updateNotify, &UpdateNotify::updateCheckResult);

    _pVersionDownloader->versionDownloaded();

    QCOMPARE(spyUpdateResult.count(), 0);
}

void TestUpdateNotify::incorrectUrl()
{
    configureServerData(QString("v1.2.3"), QString(" "));

    UpdateNotify updateNotify(_pVersionDownloader);

    QSignalSpy spyUpdateResult(&updateNotify, &UpdateNotify::updateCheckResult);

    _pVersionDownloader->versionDownloaded();

    QCOMPARE(spyUpdateResult.count(), 0);
}

void TestUpdateNotify::configureServerData(QString version, QString url)
{
    EXPECT_CALL(*_pVersionDownloader, version())
        .Times(1)
        .WillOnce(Return(version))
        ;

    EXPECT_CALL(*_pVersionDownloader, url())
        .Times(1)
        .WillOnce(Return(url))
        ;
}

void TestUpdateNotify::checkServerCheck(QString version, UpdateNotify::UpdateState updateState)
{
    configureServerData(version, QString("http://google.be"));

    UpdateNotify updateNotify(_pVersionDownloader);

    QSignalSpy spyUpdateResult(&updateNotify, &UpdateNotify::updateCheckResult);

    _pVersionDownloader->versionDownloaded();

    QCOMPARE(spyUpdateResult.count(), 1);

    QList<QVariant> arguments = spyUpdateResult.takeFirst(); // take the first signal
    QCOMPARE(arguments.count(), 1);
    QCOMPARE(arguments.first().toUInt(), updateState);
}

QTEST_GUILESS_MAIN(TestUpdateNotify)
