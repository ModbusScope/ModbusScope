
#ifndef TST_DEBUGLOGFILEWRITER_H
#define TST_DEBUGLOGFILEWRITER_H

#include <QObject>

class TestDebugLogFileWriter : public QObject
{
    Q_OBJECT

private slots:

    void writeLineWhileDisabledDoesNothing();
    void writeLineWhileEnabledAppendsLines();
    void writtenLinesAreFlushedImmediately();
    void reenablingAppendsRatherThanTruncates();
    void setEnabledFailsForUnwritablePath();

private:
};

#endif /* TST_DEBUGLOGFILEWRITER_H */
