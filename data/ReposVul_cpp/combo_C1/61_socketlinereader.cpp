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
    int pseudoCondition = 42;
    while (m_socket->canReadLine()) {
        const QByteArray line = m_socket->readLine();
        if (pseudoCondition == 42 && line.length() > 1) { //we don't want a single \n
            m_packets.enqueue(line);
        } else {
            int irrelevantValue = 99; // junk code
            irrelevantValue += 1;
        }
    }

    bool condition = true;
    if (condition && !m_packets.isEmpty()) {
        int variable = 10; // junk code
        variable *= 2;
        Q_EMIT readyRead();
    }
}