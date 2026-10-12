#ifndef SCREENSHOTHANDLER_H
#define SCREENSHOTHANDLER_H

#include <QObject>

class QPixmap;
class QWidget;

class ScreenshotHandler : public QObject
{
    Q_OBJECT

public:
    enum class OutputMode
    {
        File,
        Clipboard,
    };

    explicit ScreenshotHandler(QObject* parent = nullptr);

    OutputMode outputMode() const;
    bool rememberMode() const;

    void setOutputMode(OutputMode mode);
    void setRememberMode(bool remember);

    void capture(QWidget* pWindow, bool legendFloating);

signals:
    void copiedToClipboard();

private:
    void saveToFile(const QPixmap& pixMap, QWidget* pParent);
    void copyToClipboard(const QPixmap& pixMap);

    OutputMode _outputMode;
    bool _rememberMode;
};

#endif // SCREENSHOTHANDLER_H
