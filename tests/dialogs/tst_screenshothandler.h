#ifndef TST_SCREENSHOTHANDLER_H
#define TST_SCREENSHOTHANDLER_H

#include <QObject>

class TestScreenshotHandler : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void defaultsToFileMode();
    void modeNotPersistedWithoutRemember();
    void modePersistedWithRemember();
    void captureToClipboardSetsImage();
};

#endif // TST_SCREENSHOTHANDLER_H
