#include "tst_framingreader.h"

#include "ProtocolAdapter/framingreader.h"

#include <QSignalSpy>
#include <QTest>

static QByteArray makeFrame(const QByteArray& body)
{
    return "Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" + body;
}

void TestFramingReader::init()
{
}

void TestFramingReader::cleanup()
{
}

void TestFramingReader::singleMessage()
{
    FramingReader reader;
    QSignalSpy spy(&reader, &FramingReader::messageReceived);

    const QByteArray body = R"({"jsonrpc":"2.0","id":1,"result":{}})";
    reader.feed(makeFrame(body));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toByteArray(), body);
}

void TestFramingReader::splitAcrossFeeds()
{
    FramingReader reader;
    QSignalSpy spy(&reader, &FramingReader::messageReceived);

    const QByteArray body = R"({"id":1})";
    QByteArray frame = makeFrame(body);

    /* Split between header and body */
    int splitPos = frame.indexOf("\r\n\r\n") + 4;
    reader.feed(frame.left(splitPos));
    QCOMPARE(spy.count(), 0);

    reader.feed(frame.mid(splitPos));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toByteArray(), body);
}

void TestFramingReader::bodyChunked()
{
    FramingReader reader;
    QSignalSpy spy(&reader, &FramingReader::messageReceived);

    const QByteArray body = R"({"id":2,"result":{"status":"ok"}})";
    QByteArray frame = makeFrame(body);

    /* Feed header + separator + half the body */
    int headerEnd = frame.indexOf("\r\n\r\n") + 4;
    int halfBody = headerEnd + body.size() / 2;

    reader.feed(frame.left(halfBody));
    QCOMPARE(spy.count(), 0);

    reader.feed(frame.mid(halfBody));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toByteArray(), body);
}

void TestFramingReader::multipleMessagesOneChunk()
{
    FramingReader reader;
    QSignalSpy spy(&reader, &FramingReader::messageReceived);

    const QByteArray body1 = R"({"id":1,"result":{}})";
    const QByteArray body2 = R"({"id":2,"result":{"active":true}})";

    reader.feed(makeFrame(body1) + makeFrame(body2));

    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(0).at(0).toByteArray(), body1);
    QCOMPARE(spy.at(1).at(0).toByteArray(), body2);
}

void TestFramingReader::multipleMessagesSplit()
{
    FramingReader reader;
    QSignalSpy spy(&reader, &FramingReader::messageReceived);

    const QByteArray body1 = R"({"id":1})";
    const QByteArray body2 = R"({"id":2})";
    QByteArray combined = makeFrame(body1) + makeFrame(body2);

    /* Split somewhere in the middle of the second message */
    int splitPos = makeFrame(body1).size() + 5;
    reader.feed(combined.left(splitPos));
    QCOMPARE(spy.count(), 1);

    reader.feed(combined.mid(splitPos));
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(0).at(0).toByteArray(), body1);
    QCOMPARE(spy.at(1).at(0).toByteArray(), body2);
}

void TestFramingReader::invalidHeader()
{
    FramingReader reader;
    QSignalSpy spy(&reader, &FramingReader::messageReceived);

    /* Data with separator but no Content-Length prefix */
    reader.feed(QByteArray("X-Custom-Header: 5\r\n\r\nhello"));

    QCOMPARE(spy.count(), 0);
}

void TestFramingReader::incompleteHeader()
{
    FramingReader reader;
    QSignalSpy spy(&reader, &FramingReader::messageReceived);

    /* Only part of the header — no \r\n\r\n yet */
    reader.feed(QByteArray("Content-Length: 10"));

    QCOMPARE(spy.count(), 0);
}

QTEST_GUILESS_MAIN(TestFramingReader)
