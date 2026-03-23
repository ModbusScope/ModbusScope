#ifndef TST_FRAMINGREADER_H
#define TST_FRAMINGREADER_H

#include <QObject>

class TestFramingReader : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void singleMessage();
    void splitAcrossFeeds();
    void bodyChunked();
    void multipleMessagesOneChunk();
    void multipleMessagesSplit();
    void invalidHeader();
    void incompleteHeader();
};

#endif // TST_FRAMINGREADER_H
