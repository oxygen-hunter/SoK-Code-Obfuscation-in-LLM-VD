/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 * SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

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
    int dispatcher = 0;
    while (true) {
        switch (dispatcher) {
            case 0: {
                if (m_socket->canReadLine()) {
                    dispatcher = 1;
                    break;
                }
                dispatcher = 4;
                break;
            }
            case 1: {
                const QByteArray line = m_socket->readLine();
                dispatcher = 2;
                break;
            }
            case 2: {
                if (line.length() > 1) {
                    dispatcher = 3;
                    break;
                }
                dispatcher = 0;
                break;
            }
            case 3: {
                m_packets.enqueue(line);
                dispatcher = 0;
                break;
            }
            case 4: {
                if (!m_packets.isEmpty()) {
                    dispatcher = 5;
                    break;
                }
                dispatcher = 6;
                break;
            }
            case 5: {
                Q_EMIT readyRead();
                dispatcher = 6;
                break;
            }
            case 6: {
                return;
            }
        }
    }
}