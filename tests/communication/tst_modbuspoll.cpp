#include "tst_modbuspoll.h"

#include "communication/modbuspoll.h"
#include "models/settingsmodel.h"

#include <QLoggingCategory>
#include <QTest>

namespace {
QtMsgType& capturedType()
{
    static QtMsgType t{};
    return t;
}
QString& capturedMessage()
{
    static QString m;
    return m;
}

void captureHandler(QtMsgType type, const QMessageLogContext&, const QString& msg)
{
    capturedType() = type;
    capturedMessage() = msg;
}
} // namespace

void TestModbusPoll::init()
{
    _pSettingsModel = new SettingsModel;
    _pModbusPoll = new ModbusPoll(_pSettingsModel);
    /* Enable debug output for scope.comm.adapter so qCDebug calls reach the handler */
    QLoggingCategory::setFilterRules(QStringLiteral("scope.comm.adapter.debug=true"));
}

void TestModbusPoll::cleanup()
{
    QLoggingCategory::setFilterRules(QString());
    delete _pModbusPoll;
    delete _pSettingsModel;
}

void TestModbusPoll::diagnosticDebugLevel()
{
    capturedType() = QtMsgType{};
    capturedMessage() = QString{};
    QtMessageHandler previous = qInstallMessageHandler(captureHandler);
    _pModbusPoll->onAdapterDiagnostic(QStringLiteral("debug"), QStringLiteral("polling started"));
    qInstallMessageHandler(previous);

    QCOMPARE(capturedType(), QtDebugMsg);
    QVERIFY(capturedMessage().contains(QStringLiteral("polling started")));
}

void TestModbusPoll::diagnosticInfoLevel()
{
    capturedType() = QtMsgType{};
    capturedMessage() = QString{};
    QtMessageHandler previous = qInstallMessageHandler(captureHandler);
    _pModbusPoll->onAdapterDiagnostic(QStringLiteral("info"), QStringLiteral("session active"));
    qInstallMessageHandler(previous);

    QCOMPARE(capturedType(), QtInfoMsg);
    QVERIFY(capturedMessage().contains(QStringLiteral("session active")));
}

void TestModbusPoll::diagnosticWarningLevel()
{
    capturedType() = QtMsgType{};
    capturedMessage() = QString{};
    QtMessageHandler previous = qInstallMessageHandler(captureHandler);
    _pModbusPoll->onAdapterDiagnostic(QStringLiteral("warning"), QStringLiteral("register read failed"));
    qInstallMessageHandler(previous);

    QCOMPARE(capturedType(), QtWarningMsg);
    QVERIFY(capturedMessage().contains(QStringLiteral("register read failed")));
}

void TestModbusPoll::diagnosticErrorLevel()
{
    capturedType() = QtMsgType{};
    capturedMessage() = QString{};
    QtMessageHandler previous = qInstallMessageHandler(captureHandler);
    _pModbusPoll->onAdapterDiagnostic(QStringLiteral("error"), QStringLiteral("fatal adapter fault"));
    qInstallMessageHandler(previous);

    QCOMPARE(capturedType(), QtCriticalMsg);
    QVERIFY(capturedMessage().contains(QStringLiteral("fatal adapter fault")));
}

void TestModbusPoll::diagnosticUnknownLevel()
{
    capturedType() = QtMsgType{};
    capturedMessage() = QString{};
    QtMessageHandler previous = qInstallMessageHandler(captureHandler);
    _pModbusPoll->onAdapterDiagnostic(QStringLiteral("critical"), QStringLiteral("unexpected error"));
    qInstallMessageHandler(previous);

    QCOMPARE(capturedType(), QtWarningMsg);
    QVERIFY(capturedMessage().contains(QStringLiteral("unknown diagnostic level")));
}

QTEST_GUILESS_MAIN(TestModbusPoll)
