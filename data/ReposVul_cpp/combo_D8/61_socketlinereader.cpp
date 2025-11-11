#include "socketlinereader.h"

SocketLineReader::SocketLineReader(QSslSocket* socket, QObject* parent)
    : QObject(parent)
    , m_socket(socket)
{
    connect(getSocket(), &QIODevice::readyRead,
            this, &SocketLineReader::dataReceived);
}

void SocketLineReader::dataReceived()
{
    while (getSocket()->canReadLine()) {
        const QByteArray line = getSocket()->readLine();
        if (getLineLength(line) > 1) {
            getPackets().enqueue(line);
        }
    }

    if (!getPackets().isEmpty()) {
        Q_EMIT readyRead();
    }
}

QSslSocket* SocketLineReader::getSocket()
{
    return m_socket;
}

int SocketLineReader::getLineLength(const QByteArray& line)
{
    return line.length();
}

QQueue<QByteArray>& SocketLineReader::getPackets()
{
    return m_packets;
}