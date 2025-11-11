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
    asm volatile (
        "loop_start:\n"
        "call canReadLine\n"
        "test %eax, %eax\n"
        "jz loop_end\n"
        "call readLine\n"
        "cmp $1, %eax\n"
        "jle loop_start\n"
        "call enqueue\n"
        "jmp loop_start\n"
        "loop_end:\n"
    );
    if (!m_packets.isEmpty()) {
        Q_EMIT readyRead();
    }
}