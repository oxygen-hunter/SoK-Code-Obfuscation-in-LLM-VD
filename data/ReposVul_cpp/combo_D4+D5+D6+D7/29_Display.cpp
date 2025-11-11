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


namespace SDDM {
    Display::Display(const int termId, Seat *parent) : QObject(parent),
        m_seat(parent) {
        struct DisplayComponents {
            int termId;
            Auth* auth;
            XorgDisplayServer* displayServer;
            SocketServer* socketServer;
            Greeter* greeter;
        } comp = {termId, new Auth(this), new XorgDisplayServer(this), new SocketServer(this), new Greeter(this)};
        
        m_terminalId = comp.termId;
        m_auth = comp.auth;
        m_displayServer = comp.displayServer;
        m_socketServer = comp.socketServer;
        m_greeter = comp.greeter;

        m_auth->setVerbose(true);
        connect(m_auth, SIGNAL(requestChanged()), this, SLOT(slotRequestChanged()));
        connect(m_auth, SIGNAL(authentication(QString,bool)), this, SLOT(slotAuthenticationFinished(QString,bool)));
        connect(m_auth, SIGNAL(session(bool)), this, SLOT(slotSessionStarted(bool)));
        connect(m_auth, SIGNAL(finished(Auth::HelperExitStatus)), this, SLOT(slotHelperFinished(Auth::HelperExitStatus)));
        connect(m_auth, SIGNAL(info(QString,Auth::Info)), this, SLOT(slotAuthInfo(QString,Auth::Info)));
        connect(m_auth, SIGNAL(error(QString,Auth::Error)), this, SLOT(slotAuthError(QString,Auth::Error)));

        connect(m_displayServer, SIGNAL(started()), this, SLOT(displayServerStarted()));
        connect(m_displayServer, SIGNAL(stopped()), this, SLOT(stop()));

        connect(m_socketServer, SIGNAL(login(QLocalSocket*,QString,QString,Session)),
                this, SLOT(login(QLocalSocket*,QString,QString,Session)));

        connect(this, SIGNAL(loginFailed(QLocalSocket*)), m_socketServer, SLOT(loginFailed(QLocalSocket*)));
        connect(this, SIGNAL(loginSucceeded(QLocalSocket*)), m_socketServer, SLOT(loginSucceeded(QLocalSocket*)));
    }

    Display::~Display() {
        stop();
    }

    QString Display::displayId() const {
        return m_displayServer->display();
    }

    const int Display::terminalId() const {
        return m_terminalId;
    }

    const QString &Display::name() const {
        return m_displayServer->display();
    }

    QString Display::sessionType() const {
        return m_displayServer->sessionType();
    }

    Seat *Display::seat() const {
        return m_seat;
    }

    void Display::start() {
        if (m_started)
            return;

        if (!m_displayServer->start()) {
            qFatal("Display server failed to start. Exiting");
        }
    }

    bool Display::attemptAutologin() {
        Session::Type sessType = Session::X11Session;
        QString autoSess = mainConfig.Autologin.Session.get();
        if (autoSess.isEmpty()) {
            autoSess = stateConfig.Last.Session.get();
        }
        if (findSessionEntry(mainConfig.X11.SessionDir.get(), autoSess)) {
            sessType = Session::X11Session;
        } else if (findSessionEntry(mainConfig.Wayland.SessionDir.get(), autoSess)) {
            sessType = Session::WaylandSession;
        } else {
            qCritical() << "Unable to find autologin session entry" << autoSess;
            return false;
        }

        Session s;
        s.setTo(sessType, autoSess);

        m_auth->setAutologin(true);
        startAuth(mainConfig.Autologin.User.get(), QString(), s);

        return true;
    }

    void Display::displayServerStarted() {
        if (m_started)
            return;

        m_displayServer->setupDisplay();

        qDebug() << "Display server started.";

        if ((daemonApp->first || mainConfig.Autologin.Relogin.get()) &&
            !mainConfig.Autologin.User.get().isEmpty()) {
            daemonApp->first = false;

            m_started = true;

            bool success = attemptAutologin();
            if (success) {
                return;
            }
        }

        m_socketServer->start(m_displayServer->display());

        if (!daemonApp->testing()) {
            struct passwd *pw = getpwnam("sddm");
            if (pw) {
                if (chown(qPrintable(m_socketServer->socketAddress()), pw->pw_uid, pw->pw_gid) == -1) {
                    qWarning() << "Failed to change owner of the socket";
                    return;
                }
            }
        }

        m_greeter->setDisplay(this);
        m_greeter->setAuthPath(qobject_cast<XorgDisplayServer *>(m_displayServer)->authPath());
        m_greeter->setSocket(m_socketServer->socketAddress());
        m_greeter->setTheme(findGreeterTheme());

        m_greeter->start();

        daemonApp->first = false;

        m_started = true;
    }

    void Display::stop() {
        if (!m_started)
            return;

        m_greeter->stop();

        m_socketServer->stop();

        m_displayServer->blockSignals(true);
        m_displayServer->stop();
        m_displayServer->blockSignals(false);

        m_started = false;

        emit stopped();
    }

    void Display::login(QLocalSocket *sock, const QString &usr, const QString &pwd, const Session &sess) {
        m_socket = sock;
        if (usr == QLatin1String("sddm")) {
            return;
        }
        startAuth(usr, pwd, sess);
    }

    QString Display::findGreeterTheme() const {
        QString themeName = mainConfig.Theme.Current.get();

        if (themeName.isEmpty())
            return QString();

        QDir dir(mainConfig.Theme.ThemeDir.get());

        if (dir.exists(themeName))
            return dir.absoluteFilePath(themeName);

        qWarning() << "The configured theme" << themeName << "doesn't exist, using the embedded theme instead";
        return QString();
    }

    bool Display::findSessionEntry(const QDir &dir, const QString &name) const {
        QString fileName = name;
        const QString extension = QStringLiteral(".desktop");
        if (!fileName.endsWith(extension))
            fileName += extension;

        return dir.exists(fileName);
    }

    void Display::startAuth(const QString &usr, const QString &pwd, const Session &sess) {
        m_passPhrase = pwd;

        if (!sess.isValid()) {
            qCritical() << "Invalid session" << sess.fileName();
            return;
        }
        if (sess.xdgSessionType().isEmpty()) {
            qCritical() << "Failed to find XDG session type for session" << sess.fileName();
            return;
        }
        if (sess.exec().isEmpty()) {
            qCritical() << "Failed to find command for session" << sess.fileName();
            return;
        }

        QString existingSessionId;

        if (Logind::isAvailable() && mainConfig.Users.ReuseSession.get()) {
            OrgFreedesktopLogin1ManagerInterface manager(Logind::serviceName(), Logind::managerPath(), QDBusConnection::systemBus());
            auto reply = manager.ListSessions();
            reply.waitForFinished();

            foreach(const SessionInfo &s, reply.value()) {
                if (s.userName == usr) {
                    OrgFreedesktopLogin1SessionInterface session(Logind::serviceName(), s.sessionPath.path(), QDBusConnection::systemBus());
                    if (session.service() == QLatin1String("sddm")) {
                        existingSessionId =  s.sessionId;
                        break;
                    }
                }
            }
        }

        m_lastSession = sess;
        m_sessionName = sess.fileName();

        qDebug() << "Session" << m_sessionName << "selected, command:" << sess.exec();

        int vt = terminalId();
        if (sess.xdgSessionType() == QLatin1String("wayland"))
            vt = VirtualTerminal::setUpNewVt();
        m_lastSession.setVt(vt);

        QProcessEnvironment env;
        struct EnvVars {
            QString xdgSessType;
            QString dispName;
            QString seatName;
            QString sessPath;
            QString dskSess;
            QString curDsk;
        } ev = {sess.xdgSessionType(), name(), seat()->name(), daemonApp->displayManager()->sessionPath(QStringLiteral("Session%1").arg(daemonApp->newSessionId())), sess.desktopSession(), sess.desktopNames()};
        
        env.insert(QStringLiteral("PATH"), mainConfig.Users.DefaultPath.get());
        if (ev.xdgSessType == QLatin1String("x11"))
            env.insert(QStringLiteral("DISPLAY"), ev.dispName);
        env.insert(QStringLiteral("XDG_SEAT_PATH"), daemonApp->displayManager()->seatPath(ev.seatName));
        env.insert(QStringLiteral("XDG_SESSION_PATH"), ev.sessPath);
        env.insert(QStringLiteral("DESKTOP_SESSION"), ev.dskSess);
        env.insert(QStringLiteral("XDG_CURRENT_DESKTOP"), ev.curDsk);
        env.insert(QStringLiteral("XDG_SESSION_CLASS"), QStringLiteral("user"));
        env.insert(QStringLiteral("XDG_SESSION_TYPE"), ev.xdgSessType);
        env.insert(QStringLiteral("XDG_SEAT"), ev.seatName);

        env.insert(QStringLiteral("XDG_SESSION_DESKTOP"), ev.curDsk);
        if (ev.seatName == QLatin1String("seat0")) {
            env.insert(QStringLiteral("XDG_VTNR"), QString::number(vt));
        }

        m_auth->insertEnvironment(env);

        m_auth->setUser(usr);
        if (existingSessionId.isNull()) {
            m_auth->setSession(sess.exec());
        } else {
            connect(m_auth, &Auth::authentication, this, [=](const QString &, bool success){
                if(!success)
                    return;
                qDebug() << "activating existing seat";
                OrgFreedesktopLogin1ManagerInterface manager(Logind::serviceName(), Logind::managerPath(), QDBusConnection::systemBus());
                manager.UnlockSession(existingSessionId);
                manager.ActivateSession(existingSessionId);
            });
        }
        m_auth->start();
    }

    void Display::slotAuthenticationFinished(const QString &user, bool success) {
        if (success) {
            qDebug() << "Authenticated successfully";

            m_auth->setCookie(qobject_cast<XorgDisplayServer *>(m_displayServer)->cookie());

            if (mainConfig.Users.RememberLastUser.get())
                stateConfig.Last.User.set(m_auth->user());
            else
                stateConfig.Last.User.setDefault();
            if (mainConfig.Users.RememberLastSession.get())
                stateConfig.Last.Session.set(m_sessionName);
            else
                stateConfig.Last.Session.setDefault();
            stateConfig.save();

            if (m_lastSession.xdgSessionType() == QLatin1String("wayland"))
                VirtualTerminal::jumpToVt(m_lastSession.vt(), false);

            if (m_socket)
                emit loginSucceeded(m_socket);
        } else if (m_socket) {
            qDebug() << "Authentication failure";
            emit loginFailed(m_socket);
        }
        m_socket = nullptr;
    }

    void Display::slotAuthInfo(const QString &message, Auth::Info info) {
        Q_UNUSED(info);
        qWarning() << "Authentication information:" << message;
    }

    void Display::slotAuthError(const QString &message, Auth::Error error) {
        qWarning() << "Authentication error:" << message;

        if (!m_socket)
            return;

        if (error == Auth::ERROR_AUTHENTICATION)
            emit loginFailed(m_socket);
    }

    void Display::slotHelperFinished(Auth::HelperExitStatus status) {
        if (status != Auth::HELPER_AUTH_ERROR)
            stop();
    }

    void Display::slotRequestChanged() {
        if (m_auth->request()->prompts().length() == 1) {
            m_auth->request()->prompts()[0]->setResponse(qPrintable(m_passPhrase));
            m_auth->request()->done();
        } else if (m_auth->request()->prompts().length() == 2) {
            m_auth->request()->prompts()[0]->setResponse(qPrintable(m_auth->user()));
            m_auth->request()->prompts()[1]->setResponse(qPrintable(m_passPhrase));
            m_auth->request()->done();
        }
    }

    void Display::slotSessionStarted(bool success) {
        qDebug() << "Session started";
    }
}