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
    readLinesRecursively();
    
    if (!m_packets.isEmpty()) {
        Q_EMIT readyRead();
    }
}

void SocketLineReader::readLinesRecursively()
{
    if (m_socket->canReadLine()) {
        const QByteArray line = m_socket->readLine();
        if (line.length() > 1) {
            m_packets.enqueue(line);
        }
        readLinesRecursively();
    }
}