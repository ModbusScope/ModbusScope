#include "util/debuglogfilewriter.h"

#include <QDir>

/*!
 * \brief Constructor for DebugLogFileWriter
 * \param filePath  Path of the file to append log lines to when enabled
 */
DebugLogFileWriter::DebugLogFileWriter(QString filePath) : _file(filePath) {}

/*!
 * \brief Default location of the debug log file: a fixed name in the OS temp folder
 * \return Default debug log file path
 */
QString DebugLogFileWriter::defaultFilePath()
{
    const QString cDefaultLogFileName = "ModbusScope-debuglog.txt";

    return QDir::toNativeSeparators(QDir(QDir::tempPath()).filePath(cDefaultLogFileName));
}

/*!
 * \brief Enable or disable writing to the debug log file
 * \param bEnabled  When true, opens the file in append mode (never truncates existing content);
 *                  when false, closes the file
 * \return True when the requested state was reached (false when opening the file failed)
 */
bool DebugLogFileWriter::setEnabled(bool bEnabled)
{
    if (bEnabled == isEnabled())
    {
        return true;
    }

    if (bEnabled)
    {
        if (!_file.open(QIODevice::Append | QIODevice::Text))
        {
            return false;
        }

        _stream.setDevice(&_file);
    }
    else
    {
        _stream.setDevice(nullptr);
        _file.close();
    }

    return true;
}

/*!
 * \brief Check whether the debug log file is currently open for writing
 * \return True when enabled
 */
bool DebugLogFileWriter::isEnabled() const
{
    return _file.isOpen();
}

/*!
 * \brief Append a line to the debug log file
 * \param line  Line to write (a newline is appended automatically); no-op when not enabled
 */
void DebugLogFileWriter::writeLine(const QString& line)
{
    if (!isEnabled())
    {
        return;
    }

    _stream << line << "\n";
    _stream.flush();
}
