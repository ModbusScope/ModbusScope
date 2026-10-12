#include "dialogs/screenshothandler.h"

#include "util/fileselectionhelper.h"
#include "util/scopelogging.h"

#include <QClipboard>
#include <QFileDialog>
#include <QGuiApplication>
#include <QPixmap>
#include <QSettings>
#include <QWidget>

constexpr QLatin1String cRememberModeKey("screenshot/rememberMode");
constexpr QLatin1String cOutputModeKey("screenshot/outputMode");

//! \brief Load the remembered output mode, or default to saving to file.
ScreenshotHandler::ScreenshotHandler(QObject* parent)
    : QObject(parent), _outputMode(OutputMode::File), _rememberMode(false)
{
    const QSettings settings;
    _rememberMode = settings.value(cRememberModeKey, false).toBool();

    if (_rememberMode && settings.value(cOutputModeKey).toInt() == static_cast<int>(OutputMode::Clipboard))
    {
        _outputMode = OutputMode::Clipboard;
    }
}

//! \brief Get the active output mode.
ScreenshotHandler::OutputMode ScreenshotHandler::outputMode() const
{
    return _outputMode;
}

//! \brief Get whether the output mode is persisted across sessions.
bool ScreenshotHandler::rememberMode() const
{
    return _rememberMode;
}

//! \brief Set the active output mode and persist it when remembering is enabled.
void ScreenshotHandler::setOutputMode(OutputMode mode)
{
    if (_outputMode == mode)
    {
        return;
    }

    _outputMode = mode;

    if (_rememberMode)
    {
        QSettings settings;
        settings.setValue(cOutputModeKey, static_cast<int>(_outputMode));
    }
}

//! \brief Enable or disable persisting the output mode.
void ScreenshotHandler::setRememberMode(bool remember)
{
    _rememberMode = remember;

    QSettings settings;
    settings.setValue(cRememberModeKey, _rememberMode);
    if (_rememberMode)
    {
        settings.setValue(cOutputModeKey, static_cast<int>(_outputMode));
    }
}

/*!
 * \brief Grab the window and send it to the active output.
 * \param pWindow Window to grab.
 * \param legendFloating True when the legend is undocked, in which case it is not part of the image.
 */
void ScreenshotHandler::capture(QWidget* pWindow, bool legendFloating)
{
    if (legendFloating)
    {
        qCWarning(scopeUi) << "Undocked legend not included in screenshot";
    }

    // Grab image before showing any dialog
    const QPixmap pixMap = pWindow->window()->grab();

    if (_outputMode == OutputMode::Clipboard)
    {
        copyToClipboard(pixMap);
    }
    else
    {
        saveToFile(pixMap, pWindow);
    }
}

void ScreenshotHandler::saveToFile(const QPixmap& pixMap, QWidget* pParent)
{
    QFileDialog dialog(pParent);
    FileSelectionHelper::configureFileDialog(&dialog, FileSelectionHelper::DIALOG_TYPE_SAVE,
                                             FileSelectionHelper::FILE_TYPE_PNG);

    const QString selectedFile = FileSelectionHelper::showDialog(&dialog);
    if (!selectedFile.isEmpty())
    {
        if (!pixMap.save(selectedFile))
        {
            qCWarning(scopeUi) << "Failed to save screenshot to" << selectedFile;
        }
    }
}

void ScreenshotHandler::copyToClipboard(const QPixmap& pixMap)
{
    QGuiApplication::clipboard()->setPixmap(pixMap);
    emit copiedToClipboard();
}
