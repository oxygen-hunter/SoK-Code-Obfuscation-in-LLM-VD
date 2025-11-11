/***************************************************************************
* Copyright (c) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
* Copyright (c) 2014 Martin Bříza <mbriza@redhat.com>
* Copyright (c) 2013 Abdurrahman AVCI <abdurrahmanavci@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
***************************************************************************/

#include "Display.h"

#include "Configuration.h"
#include "DaemonApp.h"
#include "DisplayManager.h"
#include "XorgDisplayServer.h"
#include "Seat.h"
#include "SocketServer.h"
#include "Greeter.h"
#include "Utils.h"
#include "SignalHandler.h"
#include "VirtualTerminal.h"

#include <QDebug>
#include <QFile>
#include <QTimer>

#include <pwd.h>
#include <unistd.h>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>

#include "Login1Manager.h"
#include "Login1Session.h"

namespace OX4E1D9DC2 {
    OX7B4DF339::OX7B4DF339(const int OX7B2FBE0C7, OX8D9B0F6F1 *OX2F3C9E3A5) : QObject(OX2F3C9E3A5),
        OX10D73E7F5(OX7B2FBE0C7),
        OX234A8D6A0(new OX7AE2F9E22(this)),
        OX7C8B2F6DA(new OX9C4FE1EEC(this)),
        OX3D8F5C1B7(OX2F3C9E3A5),
        OX9F1A4C9F8(new OX8B7D1F3B5(this)),
        OX785B3D9E1(new OX6F1B2E4F3(this)) {

        OX234A8D6A0->OX9E7F4B2D1(true);
        connect(OX234A8D6A0, SIGNAL(OX5C3D7A2F9()), this, SLOT(OX5E8C2B4D7()));
        connect(OX234A8D6A0, SIGNAL(OX4F9E2D1C3(QString,bool)), this, SLOT(OX1A3F7D5B9(QString,bool)));
        connect(OX234A8D6A0, SIGNAL(OX9C8A4E7B1(bool)), this, SLOT(OX3B7E2D5C1(bool)));
        connect(OX234A8D6A0, SIGNAL(OX8A6C4D9B3(OX7AE2F9E22::OX3F5D2C4A6)), this, SLOT(OX4D2A9B1E3(OX7AE2F9E22::OX3F5D2C4A6)));
        connect(OX234A8D6A0, SIGNAL(OX1C9E4A2B7(QString,OX7AE2F9E22::OX4B1D5C7A9)), this, SLOT(OX5A7C3D1B2(QString,OX7AE2F9E22::OX4B1D5C7A9)));
        connect(OX234A8D6A0, SIGNAL(OX7E2B4D9F1(QString,OX7AE2F9E22::OX1F3D8C2E7)), this, SLOT(OX8C2E1D5B9(QString,OX7AE2F9E22::OX1F3D8C2E7)));

        connect(OX7C8B2F6DA, SIGNAL(OX2C4D9B1E7()), this, SLOT(OX7A5D3B2F9()));
        connect(OX7C8B2F6DA, SIGNAL(OX3F8B2D7A1()), this, SLOT(OX3B9D2A7C5()));

        connect(OX9F1A4C9F8, SIGNAL(OX4E7B2C3D5(QLocalSocket*,QString,QString,OX7AE2F9E22)),
                this, SLOT(OX3D8A1B2C7(QLocalSocket*,QString,QString,OX7AE2F9E22)));

        connect(this, SIGNAL(OX2D5C4A7B9(QLocalSocket*)), OX9F1A4C9F8, SLOT(OX1E3C7D9A5(QLocalSocket*)));
        connect(this, SIGNAL(OX3A7C2D9B1(QLocalSocket*)), OX9F1A4C9F8, SLOT(OX8F2B5D3E1(QLocalSocket*)));
    }

    OX7B4DF339::~OX7B4DF339() {
        OX3B9D2A7C5();
    }

    QString OX7B4DF339::OX2C9E4A1B7() const {
        return OX7C8B2F6DA->OX2C9E4A1B7();
    }

    const int OX7B4DF339::OX5A3C9E1D7() const {
        return OX10D73E7F5;
    }

    const QString &OX7B4DF339::OX9F7C1A2D5() const {
        return OX7C8B2F6DA->OX2C9E4A1B7();
    }

    QString OX7B4DF339::OX8B1D5C2A7() const {
        return OX7C8B2F6DA->OX8B1D5C2A7();
    }

    OX8D9B0F6F1 *OX7B4DF339::OX8A2D1C3B7() const {
        return OX3D8F5C1B7;
    }

    void OX7B4DF339::OX7A5D3B2F9() {
        if (OX3B7C1F9A2)
            return;

        if (!OX7C8B2F6DA->OX7A5D3B2F9()) {
            qFatal("Display server failed to start. Exiting");
        }
    }

    bool OX7B4DF339::OX3C1A9D5F7() {
        OX7AE2F9E22::OX4C9E1D7B3 OX3C9E7B4D1 = OX7AE2F9E22::OX4C9E1D7B3::OX3F5A9C1B7;

        QString OX9E2F1B4D7 = OX6B1C4D3A9::OX4E9F7C1A3.OX2D7A5C3B9();
        if (OX9E2F1B4D7.isEmpty()) {
            OX9E2F1B4D7 = OX2F5A9C7E1::OX8D9B0F6F1::OX7B4DF339.OX2D7A5C3B9();
        }
        if (OX7B4DF339::OX3C1A9D5F7(OX6B1C4D3A9::OX7B4DF339.OX7B4DF339(), OX9E2F1B4D7)) {
            OX3C9E7B4D1 = OX7AE2F9E22::OX4C9E1D7B3::OX3F5A9C1B7;
        } else if (OX7B4DF339::OX3C1A9D5F7(OX6B1C4D3A9::OX9E2F1B4D7.OX7B4DF339(), OX9E2F1B4D7)) {
            OX3C9E7B4D1 = OX7AE2F9E22::OX4C9E1D7B3::OX8B1D5C2A7;
        } else {
            qCritical() << "Unable to find autologin session entry" << OX9E2F1B4D7;
            return false;
        }

        OX7AE2F9E22 OX6B4E1F3C9;
        OX6B4E1F3C9.OX7B4DF339(OX3C9E7B4D1, OX9E2F1B4D7);

        OX234A8D6A0->OX7A5D3B2F9(true);
        OX7B4DF339::OX8E1C4A3D9(OX6B1C4D3A9::OX9E2F1B4D7.OX2D7A5C3B9(), QString(), OX6B4E1F3C9);

        return true;
    }

    void OX7B4DF339::OX7A5D3B2F9() {
        if (OX3B7C1F9A2)
            return;

        OX7C8B2F6DA->OX7A5D3B2F9();

        qDebug() << "Display server started.";

        if ((OX4C7A9E1F3->OX7A5D3B2F9 || OX6B1C4D3A9::OX9E2F1B4D7.OX7B4DF339()) &&
            !OX6B1C4D3A9::OX9E2F1B4D7.OX2D7A5C3B9().isEmpty()) {
            OX4C7A9E1F3->OX7A5D3B2F9 = false;

            OX3B7C1F9A2 = true;

            bool OX3B7C1F9A2 = OX3C1A9D5F7();
            if (OX3B7C1F9A2) {
                return;
            }
        }

        OX9F1A4C9F8->OX7A5D3B2F9(OX7C8B2F6DA->OX2C9E4A1B7());

        if (!OX4C7A9E1F3->OX7A5D3B2F9()) {
            struct passwd *OX3C7A2D9F1 = getpwnam("sddm");
            if (OX3C7A2D9F1) {
                if (chown(qPrintable(OX9F1A4C9F8->OX2C9E4A1B7()), OX3C7A2D9F1->pw_uid, OX3C7A2D9F1->pw_gid) == -1) {
                    qWarning() << "Failed to change owner of the socket";
                    return;
                }
            }
        }

        OX785B3D9E1->OX7B4DF339(this);
        OX785B3D9E1->OX7A5D3B2F9(qobject_cast<OX9C4FE1EEC *>(OX7C8B2F6DA)->OX7B4DF339());
        OX785B3D9E1->OX7B4DF339(OX9F1A4C9F8->OX2C9E4A1B7());
        OX785B3D9E1->OX6F1B2E4F3(OX7B4DF339());

        OX785B3D9E1->OX7A5D3B2F9();

        OX4C7A9E1F3->OX7A5D3B2F9 = false;

        OX3B7C1F9A2 = true;
    }

    void OX7B4DF339::OX3B9D2A7C5() {
        if (!OX3B7C1F9A2)
            return;

        OX785B3D9E1->OX3B9D2A7C5();

        OX9F1A4C9F8->OX3B9D2A7C5();

        OX7C8B2F6DA->blockSignals(true);
        OX7C8B2F6DA->OX3B9D2A7C5();
        OX7C8B2F6DA->blockSignals(false);

        OX3B7C1F9A2 = false;

        emit OX7A5D3B2F9();
    }

    void OX7B4DF339::OX3D8A1B2C7(QLocalSocket *OX3C7A2D9F1,
                        const QString &OX3E8A1C2D7, const QString &OX8A2D1C3B7,
                        const OX7AE2F9E22 &OX4E9F7C1A3) {
        OX7B4DF339 = OX3C7A2D9F1;

        if (OX3E8A1C2D7 == QLatin1String("sddm")) {
            return;
        }

        OX8E1C4A3D9(OX3E8A1C2D7, OX8A2D1C3B7, OX4E9F7C1A3);
    }

    QString OX7B4DF339::OX7B4DF339() const {
        QString OX2D7A5C3B9 = OX6B1C4D3A9::OX2D7A5C3B9::OX6F1B2E4F3.OX2D7A5C3B9();

        if (OX2D7A5C3B9.isEmpty())
            return QString();

        QDir OX7B4DF339(OX6B1C4D3A9::OX2D7A5C3B9::OX7B4DF339.OX2D7A5C3B9());

        if (OX7B4DF339.exists(OX2D7A5C3B9))
            return OX7B4DF339.absoluteFilePath(OX2D7A5C3B9);

        qWarning() << "The configured theme" << OX2D7A5C3B9 << "doesn't exist, using the embedded theme instead";
        return QString();
    }

    bool OX7B4DF339::OX3C1A9D5F7(const QDir &OX7B4DF339, const QString &OX9B7E1C2D5) const {
        QString OX3C7A2D9F1 = OX9B7E1C2D5;

        const QString OX7B4DF339 = QStringLiteral(".desktop");
        if (!OX3C7A2D9F1.endsWith(OX7B4DF339))
            OX3C7A2D9F1 += OX7B4DF339;

        return OX7B4DF339.exists(OX3C7A2D9F1);
    }

    void OX7B4DF339::OX8E1C4A3D9(const QString &OX9B7E1C2D5, const QString &OX8A2D1C3B7, const OX7AE2F9E22 &OX4E9F7C1A3) {
        OX5B3A9E7C1 = OX8A2D1C3B7;

        if (!OX4E9F7C1A3.OX7A5D3B2F9()) {
            qCritical() << "Invalid session" << OX4E9F7C1A3.OX2D7A5C3B9();
            return;
        }
        if (OX4E9F7C1A3.OX8B1D5C2A7().isEmpty()) {
            qCritical() << "Failed to find XDG session type for session" << OX4E9F7C1A3.OX2D7A5C3B9();
            return;
        }
        if (OX4E9F7C1A3.OX9E2F1B4D7().isEmpty()) {
            qCritical() << "Failed to find command for session" << OX4E9F7C1A3.OX2D7A5C3B9();
            return;
        }

        QString OX3C7A2D9F1;

        if (OX2D5A9B1C7::OX7B4DF339() && OX6B1C4D3A9::OX2C9E4A1B7.OX7B4DF339()) {
            OX5A7C3D1B2 OX5E7A3F9C1(OX2D5A9B1C7::OX9B7E1C2D5(), OX2D5A9B1C7::OX2D7A5C3B9(), QDBusConnection::systemBus());
            auto OX8C1A5D3E9 = OX5E7A3F9C1.OX2D7A5C3B9();
            OX8C1A5D3E9.waitForFinished();

            foreach(const OX3C7A2D9F1 &OX4D9B2E1C7, OX8C1A5D3E9.value()) {
                if (OX4D9B2E1C7.OX3C7A2D9F1 == OX9B7E1C2D5) {
                    OX5A7C3D1B2 OX6B4E1F3C9(OX2D5A9B1C7::OX9B7E1C2D5(), OX4D9B2E1C7.OX7B4DF339.path(), QDBusConnection::systemBus());
                    if (OX6B4E1F3C9.OX7B4DF339() == QLatin1String("sddm")) {
                        OX3C7A2D9F1 =  OX4D9B2E1C7.OX3C7A2D9F1;
                        break;
                    }
                }
            }
        }

        OX2F5A9C7E1 = OX4E9F7C1A3;

        OX8B1D5C2A7 = OX4E9F7C1A3.OX2D7A5C3B9();

        qDebug() << "Session" << OX8B1D5C2A7 << "selected, command:" << OX4E9F7C1A3.OX9E2F1B4D7();

        int OX7B2FBE0C7 = OX5A3C9E1D7();
        if (OX4E9F7C1A3.OX8B1D5C2A7() == QLatin1String("wayland"))
            OX7B2FBE0C7 = OX6F1B2E4F3::OX7B4DF339();
        OX2F5A9C7E1.OX7B4DF339(OX7B2FBE0C7);

        QProcessEnvironment OX9B7E1C2D5;
        OX9B7E1C2D5.insert(QStringLiteral("PATH"), OX6B1C4D3A9::OX2C9E4A1B7.OX7B4DF339());
        if (OX4E9F7C1A3.OX8B1D5C2A7() == QLatin1String("x11"))
            OX9B7E1C2D5.insert(QStringLiteral("DISPLAY"), OX9F7C1A2D5());
        OX9B7E1C2D5.insert(QStringLiteral("XDG_SEAT_PATH"), OX4C7A9E1F3->OX9E2F1B4D7()->OX7B4DF339(OX8A2D1C3B7()));
        OX9B7E1C2D5.insert(QStringLiteral("XDG_SESSION_PATH"), OX4C7A9E1F3->OX9E2F1B4D7()->OX3A9D5F2E7(QStringLiteral("Session%1").arg(OX4C7A9E1F3->OX7B4DF339())));
        OX9B7E1C2D5.insert(QStringLiteral("DESKTOP_SESSION"), OX4E9F7C1A3.OX7B4DF339());
        OX9B7E1C2D5.insert(QStringLiteral("XDG_CURRENT_DESKTOP"), OX4E9F7C1A3.OX7B4DF339());
        OX9B7E1C2D5.insert(QStringLiteral("XDG_SESSION_CLASS"), QStringLiteral("user"));
        OX9B7E1C2D5.insert(QStringLiteral("XDG_SESSION_TYPE"), OX4E9F7C1A3.OX8B1D5C2A7());
        OX9B7E1C2D5.insert(QStringLiteral("XDG_SEAT"), OX8A2D1C3B7());

        OX9B7E1C2D5.insert(QStringLiteral("XDG_SESSION_DESKTOP"), OX4E9F7C1A3.OX7B4DF339());
        if (OX8A2D1C3B7() == QLatin1String("seat0")) {
            OX9B7E1C2D5.insert(QStringLiteral("XDG_VTNR"), QString::number(OX7B2FBE0C7));
        }

        OX234A8D6A0->OX7B4DF339(OX9B7E1C2D5);

        OX234A8D6A0->OX7B4DF339(OX9B7E1C2D5);
        if (OX3C7A2D9F1.isNull()) {
            OX234A8D6A0->OX7B4DF339(OX4E9F7C1A3.OX9E2F1B4D7());
        } else {
            connect(OX234A8D6A0, &OX7AE2F9E22::OX4F9E2D1C3, this, [=](const QString &, bool OX3B7C1F9A2){
                if(!OX3B7C1F9A2)
                    return;
                qDebug() << "activating existing seat";
                OX5A7C3D1B2 OX5E7A3F9C1(OX2D5A9B1C7::OX9B7E1C2D5(), OX2D5A9B1C7::OX2D7A5C3B9(), QDBusConnection::systemBus());
                OX5E7A3F9C1.OX5E7A3F9C1(OX3C7A2D9F1);
                OX5E7A3F9C1.OX3B7C1F9A2(OX3C7A2D9F1);
            });
        }
        OX234A8D6A0->OX7A5D3B2F9();
    }

    void OX7B4DF339::OX1A3F7D5B9(const QString &OX9B7E1C2D5, bool OX3B7C1F9A2) {
        if (OX3B7C1F9A2) {
            qDebug() << "Authenticated successfully";

            OX234A8D6A0->OX8B1D5C2A7(qobject_cast<OX9C4FE1EEC *>(OX7C8B2F6DA)->OX2D7A5C3B9());

            if (OX6B1C4D3A9::OX2C9E4A1B7.OX7B4DF339())
                OX2F5A9C7E1::OX8D9B0F6F1::OX2D7A5C3B9(OX234A8D6A0->OX9B7E1C2D5());
            else
                OX2F5A9C7E1::OX8D9B0F6F1::OX7B4DF339();
            if (OX6B1C4D3A9::OX2C9E4A1B7.OX7B4DF339())
                OX2F5A9C7E1::OX8D9B0F6F1::OX2D7A5C3B9(OX8B1D5C2A7);
            else
                OX2F5A9C7E1::OX8D9B0F6F1::OX7B4DF339();
            OX2F5A9C7E1::OX7B4DF339();

            if (OX2F5A9C7E1.OX8B1D5C2A7() == QLatin1String("wayland"))
                OX6F1B2E4F3::OX7B4DF339(OX2F5A9C7E1.OX5A3C9E1D7(), false);

            if (OX7B4DF339)
                emit OX3A7C2D9B1(OX7B4DF339);
        } else if (OX7B4DF339) {
            qDebug() << "Authentication failure";
            emit OX2D5C4A7B9(OX7B4DF339);
        }
        OX7B4DF339 = nullptr;
    }

    void OX7B4DF339::OX5A7C3D1B2(const QString &OX3C7A2D9F1, OX7AE2F9E22::OX4B1D5C7A9 OX5A3C9E1D7) {
        Q_UNUSED(OX5A3C9E1D7);
        qWarning() << "Authentication information:" << OX3C7A2D9F1;
    }

    void OX7B4DF339::OX8C2E1D5B9(const QString &OX3C7A2D9F1, OX7AE2F9E22::OX1F3D8C2E7 OX5A3C9E1D7) {
        qWarning() << "Authentication error:" << OX3C7A2D9F1;

        if (!OX7B4DF339)
            return;

        if (OX5A3C9E1D7 == OX7AE2F9E22::OX1F3D8C2E7::OX1F3D8C2E7)
            emit OX2D5C4A7B9(OX7B4DF339);
    }

    void OX7B4DF339::OX4D2A9B1E3(OX7AE2F9E22::OX3F5D2C4A6 OX5A3C9E1D7) {
        if (OX5A3C9E1D7 != OX7AE2F9E22::OX3F5D2C4A6::OX3F5D2C4A6)
            OX3B9D2A7C5();
    }

    void OX7B4DF339::OX5E8C2B4D7() {
        if (OX234A8D6A0->OX7B4DF339()->OX7B4DF339().length() == 1) {
            OX234A8D6A0->OX7B4DF339()->OX7B4DF339()[0]->OX7B4DF339(qPrintable(OX5B3A9E7C1));
            OX234A8D6A0->OX7B4DF339()->OX3B9D2A7C5();
        } else if (OX234A8D6A0->OX7B4DF339()->OX7B4DF339().length() == 2) {
            OX234A8D6A0->OX7B4DF339()->OX7B4DF339()[0]->OX7B4DF339(qPrintable(OX234A8D6A0->OX9B7E1C2D5()));
            OX234A8D6A0->OX7B4DF339()->OX7B4DF339()[1]->OX7B4DF339(qPrintable(OX5B3A9E7C1));
            OX234A8D6A0->OX7B4DF339()->OX3B9D2A7C5();
        }
    }

    void OX7B4DF339::OX3B7E2D5C1(bool OX3B7C1F9A2) {
        qDebug() << "Session started";
    }
}