
#ifndef TST_DEBUGLOGFILEWRITER_H
#define TST_DEBUGLOGFILEWRITER_H

#include <QObject>
#include <QTemporaryDir>

class TestDebugLogFileWriter : public QObject
{
    Q_OBJECT

private slots:

    void init();
    void cleanup();

    void writeLineWhileDisabledDoesNothing();
    void writeLineWhileEnabledAppendsLines();
    void writtenLinesAreFlushedImmediately();
    void reenablingAppendsRatherThanTruncates();

private:
    QTemporaryDir* _pTempDir;
};

#endif /* TST_DEBUGLOGFILEWRITER_H */
