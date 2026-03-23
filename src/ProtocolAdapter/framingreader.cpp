#include "ProtocolAdapter/framingreader.h"

#include "util/scopelogging.h"

FramingReader::FramingReader(QObject* parent) : QObject(parent)
{
}

void FramingReader::feed(const QByteArray& data)
{
    _buffer.append(data);
    tryParse();
}

void FramingReader::tryParse()
{
    while (true)
    {
        if (_state == State::HEADER)
        {
            const QByteArray separator = "\r\n\r\n";
            int separatorPos = _buffer.indexOf(separator);
            if (separatorPos < 0)
            {
                return;
            }

            QByteArray header = _buffer.left(separatorPos);
            const QByteArray prefix = "Content-Length: ";
            if (!header.startsWith(prefix))
            {
                qCWarning(scopeComm) << "FramingReader: unexpected header:" << header;
                _buffer.clear();
                return;
            }

            bool ok = false;
            _pendingBodySize = header.mid(prefix.size()).toInt(&ok);
            if (!ok || _pendingBodySize < 0)
            {
                qCWarning(scopeComm) << "FramingReader: invalid Content-Length:" << header;
                _buffer.clear();
                return;
            }

            _buffer.remove(0, separatorPos + separator.size());
            _state = State::BODY;
        }

        if (_state == State::BODY)
        {
            if (_buffer.size() < _pendingBodySize)
            {
                return;
            }

            QByteArray body = _buffer.left(_pendingBodySize);
            _buffer.remove(0, _pendingBodySize);
            _state = State::HEADER;
            _pendingBodySize = 0;

            emit messageReceived(body);
        }
    }
}
