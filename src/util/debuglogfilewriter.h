#ifndef DEBUGLOGFILEWRITER_H
#define DEBUGLOGFILEWRITER_H

#include <QFile>
#include <QString>
#include <QTextStream>

class DebugLogFileWriter
{
public:
    explicit DebugLogFileWriter(QString filePath = defaultFilePath());

    static QString defaultFilePath();

    bool setEnabled(bool bEnabled);
    bool isEnabled() const;

    void writeLine(const QString& line);

private:
    QFile _file;
    QTextStream _stream;
};

#endif // DEBUGLOGFILEWRITER_H
