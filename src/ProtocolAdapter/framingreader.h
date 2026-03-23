#ifndef FRAMINGREADER_H
#define FRAMINGREADER_H

#include <QByteArray>
#include <QObject>

/*!
 * \brief Parses Content-Length framed messages from a raw byte stream.
 *
 * Feed incoming bytes via feed(). Complete messages are emitted via messageReceived().
 * Expected framing format: \c "Content-Length: N\r\n\r\n<N bytes of UTF-8 JSON>"
 */
class FramingReader : public QObject
{
    Q_OBJECT

public:
    explicit FramingReader(QObject* parent = nullptr);

    /*!
     * \brief Append incoming bytes to the internal buffer and parse any complete messages.
     * \param data Raw bytes received from the adapter process stdout.
     */
    void feed(const QByteArray& data);

signals:
    /*!
     * \brief Emitted when a complete framed message body has been parsed.
     * \param body The raw JSON body (without the Content-Length header).
     */
    void messageReceived(QByteArray body);

private:
    enum class State
    {
        HEADER,
        BODY
    };

    void tryParse();

    QByteArray _buffer;
    State _state{ State::HEADER };
    int _pendingBodySize{ 0 };
};

#endif // FRAMINGREADER_H
