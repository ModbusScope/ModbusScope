#include "tst_screenshothandler.h"

#include "dialogs/screenshothandler.h"

#include <QClipboard>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QSettings>
#include <QSignalSpy>
#include <QTest>
#include <QWidget>

using OutputMode = ScreenshotHandler::OutputMode;

void TestScreenshotHandler::initTestCase()
{
    QCoreApplication::setOrganizationName("ModbusScopeTest");
    QCoreApplication::setApplicationName("tst_screenshothandler");
}

void TestScreenshotHandler::init()
{
    QSettings().clear();
}

void TestScreenshotHandler::cleanup()
{
    QSettings().clear();
}

void TestScreenshotHandler::defaultsToFileMode()
{
    const ScreenshotHandler handler;

    QCOMPARE(handler.outputMode(), OutputMode::File);
    QVERIFY(!handler.rememberMode());
}

void TestScreenshotHandler::modeNotPersistedWithoutRemember()
{
    {
        ScreenshotHandler handler;
        handler.setOutputMode(OutputMode::Clipboard);
    }

    const ScreenshotHandler handler;
    QCOMPARE(handler.outputMode(), OutputMode::File);
}

void TestScreenshotHandler::modePersistedWithRemember()
{
    {
        ScreenshotHandler handler;
        handler.setRememberMode(true);
        handler.setOutputMode(OutputMode::Clipboard);
    }

    const ScreenshotHandler handler;
    QVERIFY(handler.rememberMode());
    QCOMPARE(handler.outputMode(), OutputMode::Clipboard);
}

void TestScreenshotHandler::captureToClipboardSetsImage()
{
    QWidget widget;
    widget.resize(50, 40);
    widget.show();

    QGuiApplication::clipboard()->clear();

    ScreenshotHandler handler;
    handler.setOutputMode(OutputMode::Clipboard);
    handler.capture(&widget, false);

    QVERIFY(!QGuiApplication::clipboard()->image().isNull());
}

void TestScreenshotHandler::captureToClipboardEmitsCopied()
{
    QWidget widget;
    widget.resize(50, 40);
    widget.show();

    ScreenshotHandler handler;
    handler.setOutputMode(OutputMode::Clipboard);

    const QSignalSpy spy(&handler, &ScreenshotHandler::copiedToClipboard);
    handler.capture(&widget, false);

    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(TestScreenshotHandler)
