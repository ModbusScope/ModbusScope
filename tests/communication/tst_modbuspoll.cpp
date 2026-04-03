#include "tst_modbuspoll.h"

#include "communication/modbuspoll.h"
#include "models/settingsmodel.h"

#include <QLoggingCategory>
#include <QTest>

namespace
{
    QtMsgType g_capturedType{};
    QString g_capturedMessage{};

    void captureHandler(QtMsgType type, const QMessageLogContext&, const QString& msg)
    {
        g_capturedType = type;
        g_capturedMessage = msg;
    }
}

void TestModbusPoll::init()
{
    _pSettingsModel = new SettingsModel;
    _pModbusPoll = new ModbusPoll(_pSettingsModel);
    /* Enable debug output for scope.comm so qCDebug calls reach the handler */
    QLoggingCategory::setFilterRules(QStringLiteral("scope.comm.debug=true"));
}

void TestModbusPoll::cleanup()
{
    QLoggingCategory::setFilterRules(QString());
    delete _pModbusPoll;
    delete _pSettingsModel;
}

void TestModbusPoll::diagnosticDebugLevel()
{
    QtMessageHandler previous = qInstallMessageHandler(captureHandler);
    bool invoked =
      QMetaObject::invokeMethod(_pModbusPoll, "onAdapterDiagnostic", Q_ARG(QString, QStringLiteral("debug")),
                                Q_ARG(QString, QStringLiteral("polling started")));
    QVERIFY(invoked);
    qInstallMessageHandler(previous);

    QCOMPARE(g_capturedType, QtDebugMsg);
    QVERIFY(g_capturedMessage.contains(QStringLiteral("polling started")));
}

void TestModbusPoll::diagnosticInfoLevel()
{
    QtMessageHandler previous = qInstallMessageHandler(captureHandler);
    QMetaObject::invokeMethod(_pModbusPoll, "onAdapterDiagnostic",
                              Q_ARG(QString, QStringLiteral("info")),
                              Q_ARG(QString, QStringLiteral("session active")));
    qInstallMessageHandler(previous);

    QCOMPARE(g_capturedType, QtInfoMsg);
    QVERIFY(g_capturedMessage.contains(QStringLiteral("session active")));
}

void TestModbusPoll::diagnosticWarningLevel()
{
    QtMessageHandler previous = qInstallMessageHandler(captureHandler);
    QMetaObject::invokeMethod(_pModbusPoll, "onAdapterDiagnostic",
                              Q_ARG(QString, QStringLiteral("warning")),
                              Q_ARG(QString, QStringLiteral("register read failed")));
    qInstallMessageHandler(previous);

    QCOMPARE(g_capturedType, QtWarningMsg);
    QVERIFY(g_capturedMessage.contains(QStringLiteral("register read failed")));
}

void TestModbusPoll::diagnosticUnknownLevel()
{
    QtMessageHandler previous = qInstallMessageHandler(captureHandler);
    QMetaObject::invokeMethod(_pModbusPoll, "onAdapterDiagnostic",
                              Q_ARG(QString, QStringLiteral("critical")),
                              Q_ARG(QString, QStringLiteral("unexpected error")));
    qInstallMessageHandler(previous);

    QCOMPARE(g_capturedType, QtWarningMsg);
    QVERIFY(g_capturedMessage.contains(QStringLiteral("unknown diagnostic level")));
}

QTEST_GUILESS_MAIN(TestModbusPoll)
