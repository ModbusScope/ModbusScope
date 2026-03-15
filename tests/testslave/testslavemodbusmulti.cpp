#include "testslavemodbusmulti.h"

#include <QHostAddress>
#include <QModbusDataUnit>

/* Modbus TCP frame layout:
 *   Bytes 0-1 : Transaction Identifier
 *   Bytes 2-3 : Protocol Identifier (0x0000)
 *   Bytes 4-5 : Length (number of bytes that follow, including Unit ID)
 *   Byte  6   : Unit Identifier (slave ID)
 *   Byte  7+  : PDU (function code + data)
 */
static const int MBAP_SIZE = 6;

TestSlaveMultiModbus::TestSlaveMultiModbus(QObject* parent) : QTcpServer(parent)
{
}

TestSlaveMultiModbus::~TestSlaveMultiModbus()
{
    qDeleteAll(_devices);
}

bool TestSlaveMultiModbus::listenOnPort(quint16 port)
{
    return listen(QHostAddress::LocalHost, port);
}

void TestSlaveMultiModbus::stopListening()
{
    for (auto* socket : std::as_const(_sockets))
    {
        socket->disconnectFromHost();
    }
    _sockets.clear();
    close();
}

TestDevice* TestSlaveMultiModbus::testDevice(quint8 slaveId)
{
    if (!_devices.contains(slaveId))
    {
        _devices.insert(slaveId, new TestDevice(this));
    }
    return _devices.value(slaveId);
}

void TestSlaveMultiModbus::incomingConnection(qintptr socketDescriptor)
{
    auto* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, &QTcpSocket::readyRead, this, &TestSlaveMultiModbus::handleReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, [this, socket]() {
        _sockets.removeOne(socket);
        socket->deleteLater();
    });
    _sockets.append(socket);
}

void TestSlaveMultiModbus::handleReadyRead()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket)
    {
        return;
    }

    while (socket->bytesAvailable() >= MBAP_SIZE)
    {
        const QByteArray header = socket->peek(MBAP_SIZE);
        const quint16 length = (static_cast<quint8>(header[4]) << 8) | static_cast<quint8>(header[5]);

        if (socket->bytesAvailable() < MBAP_SIZE + length)
        {
            break;
        }

        const QByteArray frame = socket->read(MBAP_SIZE + length); // NOLINT(clang-analyzer-security.insecureAPI*)
        processFrame(socket, frame);
    }
}

void TestSlaveMultiModbus::processFrame(QTcpSocket* socket, const QByteArray& frame)
{
    /* Minimum: MBAP (6) + unit ID (1) + function code (1) + 2-byte address + 2-byte quantity */
    if (frame.size() < MBAP_SIZE + 6)
    {
        return;
    }

    const quint8 unitId    = static_cast<quint8>(frame[6]);
    const quint8 fc        = static_cast<quint8>(frame[7]);
    const quint16 startAddr = (static_cast<quint8>(frame[8]) << 8) | static_cast<quint8>(frame[9]);
    const quint16 quantity  = (static_cast<quint8>(frame[10]) << 8) | static_cast<quint8>(frame[11]);

    /* Only Read Holding Registers (FC 0x03) is implemented */
    if (fc != 0x03)
    {
        /* Modbus exception response: error FC + exception code 0x01 (Illegal Function) */
        const quint16 excLength = 3u; /* unit ID (1) + error FC (1) + exception code (1) */
        QByteArray excResponse(MBAP_SIZE + excLength, 0x00);
        excResponse[0] = frame[0]; excResponse[1] = frame[1]; /* Transaction ID */
        excResponse[4] = static_cast<char>((excLength >> 8) & 0xFF);
        excResponse[5] = static_cast<char>(excLength & 0xFF);
        excResponse[6] = static_cast<char>(unitId);
        excResponse[7] = static_cast<char>(fc | 0x80);
        excResponse[8] = 0x01; /* Illegal Function */
        socket->write(excResponse);
        return;
    }

    static const quint16 maxQuantity = 125u; /* Modbus FC 0x03 max registers per request */
    if (quantity > maxQuantity)
    {
        return;
    }

    /* Response length field: unit ID (1) + FC (1) + byte count (1) + data (2*qty) */
    const quint32 respLength = 3u + quantity * 2u;

    QByteArray response;
    response.resize(MBAP_SIZE + respLength);

    response[0] = frame[0]; response[1] = frame[1]; /* Transaction ID */
    response[2] = 0x00;     response[3] = 0x00;     /* Protocol ID   */
    response[4] = static_cast<char>((respLength >> 8) & 0xFF);
    response[5] = static_cast<char>(respLength & 0xFF);
    response[6] = static_cast<char>(unitId);
    response[7] = static_cast<char>(fc);
    response[8] = static_cast<char>((quantity * 2u) & 0xFF); /* Byte count */

    TestDevice* device = _devices.value(unitId, nullptr);
    for (quint16 i = 0; i < quantity; i++)
    {
        quint16 value = 0;
        if (device)
        {
            auto* slaveData = device->slaveData(QModbusDataUnit::HoldingRegisters);
            if (slaveData && slaveData->registerState(startAddr + i))
            {
                value = slaveData->registerValue(startAddr + i);
            }
        }
        response[9 + i * 2]     = static_cast<char>((value >> 8) & 0xFF);
        response[9 + i * 2 + 1] = static_cast<char>(value & 0xFF);
    }

    socket->write(response);
}
