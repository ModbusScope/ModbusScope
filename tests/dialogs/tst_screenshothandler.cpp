#include "tst_screenshothandler.h"

#include "dialogs/screenshothandler.h"

#include <QClipboard>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QSettings>
#include <QTest>
#include <QWidget>

using OutputMode = ScreenshotHandler::OutputMode;

void TestScreenshotHandler::init()
{
    QCoreApplication::setOrganizationName("ModbusScopeTest");
    QCoreApplication::setApplicationName("tst_screenshothandler");
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

QTEST_MAIN(TestScreenshotHandler)
