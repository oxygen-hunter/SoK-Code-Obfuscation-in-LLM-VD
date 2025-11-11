#include "socketlinereader.h"

struct SocketData {
    QObject* p;
    QSslSocket* s;
};

SocketLineReader::SocketLineReader(QSslSocket* socket, QObject* parent)
    : QObject(parent)
{
    SocketData data = {parent, socket};
    connect(data.s, &QIODevice::readyRead,
            this, &SocketLineReader::dataReceived);
    m_socket = data.s;
}

void SocketLineReader::dataReceived()
{
    QByteArray a = m_socket->readLine();
    QByteArray b = a;
    while (m_socket->canReadLine()) {
        if (b.length() > 1) {
            m_packets.enqueue(b);
        }
        a = m_socket->readLine();
        b = a;
    }

    if (!m_packets.isEmpty()) {
        Q_EMIT readyRead();
    }
}