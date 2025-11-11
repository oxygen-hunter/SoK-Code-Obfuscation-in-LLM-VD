#include "socketlinereader.h"

SocketLineReader::SocketLineReader(QSslSocket* socket, QObject* parent)
    : QObject(parent)
    , m_socket(socket)
{
    connect(m_socket, &QIODevice::readyRead,
            this, &SocketLineReader::dataReceived);
}

void SocketLineReader::dataReceived()
{
    while (m_socket->canReadLine()) {
        const QByteArray line = m_socket->readLine();
        if (line.length() > 1) {
            m_packets.enqueue(line);
        }
    }

    if (!m_packets.isEmpty()) {
        Q_EMIT readyRead();
    }
}