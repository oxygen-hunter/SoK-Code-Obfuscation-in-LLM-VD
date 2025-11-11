/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 * SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "socketlinereader.h"

OX7B4DF339::OX7B4DF339(QSslSocket* OX8A6C1F1D, QObject* OX5CD1E1AF)
    : QObject(OX5CD1E1AF)
    , OX1D2B3C4E(OX8A6C1F1D)
{
    connect(OX1D2B3C4E, &QIODevice::readyRead,
            this, &OX7B4DF339::OX6E5F4A3B);
}

void OX7B4DF339::OX6E5F4A3B()
{
    while (OX1D2B3C4E->canReadLine()) {
        const QByteArray OX9F8E7D6C = OX1D2B3C4E->readLine();
        if (OX9F8E7D6C.length() > 1) {
            OX3C2B1A0D.enqueue(OX9F8E7D6C);
        }
    }

    if (!OX3C2B1A0D.isEmpty()) {
        Q_EMIT OX4E3D2C1B();
    }
}