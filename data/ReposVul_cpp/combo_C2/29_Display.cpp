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
    Display::Display(const int terminalId, Seat *parent) : QObject(parent),
        m_terminalId(terminalId),
        m_auth(new Auth(this)),
        m_displayServer(new XorgDisplayServer(this)),
        m_seat(parent),
        m_socketServer(new SocketServer(this)),
        m_greeter(new Greeter(this)) {

        int dispatcher = 0;
        while (dispatcher >= 0) {
            switch (dispatcher) {
                case 0:
                    m_auth->setVerbose(true);
                    connect(m_auth, SIGNAL(requestChanged()), this, SLOT(slotRequestChanged()));
                    connect(m_auth, SIGNAL(authentication(QString,bool)), this, SLOT(slotAuthenticationFinished(QString,bool)));
                    connect(m_auth, SIGNAL(session(bool)), this, SLOT(slotSessionStarted(bool)));
                    connect(m_auth, SIGNAL(finished(Auth::HelperExitStatus)), this, SLOT(slotHelperFinished(Auth::HelperExitStatus)));
                    connect(m_auth, SIGNAL(info(QString,Auth::Info)), this, SLOT(slotAuthInfo(QString,Auth::Info)));
                    connect(m_auth, SIGNAL(error(QString,Auth::Error)), this, SLOT(slotAuthError(QString,Auth::Error)));
                    dispatcher = 1;
                    break;
                case 1:
                    connect(m_displayServer, SIGNAL(started()), this, SLOT(displayServerStarted()));
                    connect(m_displayServer, SIGNAL(stopped()), this, SLOT(stop()));
                    dispatcher = 2;
                    break;
                case 2:
                    connect(m_socketServer, SIGNAL(login(QLocalSocket*,QString,QString,Session)),
                            this, SLOT(login(QLocalSocket*,QString,QString,Session)));
                    dispatcher = 3;
                    break;
                case 3:
                    connect(this, SIGNAL(loginFailed(QLocalSocket*)), m_socketServer, SLOT(loginFailed(QLocalSocket*)));
                    connect(this, SIGNAL(loginSucceeded(QLocalSocket*)), m_socketServer, SLOT(loginSucceeded(QLocalSocket*)));
                    dispatcher = -1;
                    break;
            }
        }
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
        int dispatcher = 0;
        while (dispatcher >= 0) {
            switch (dispatcher) {
                case 0:
                    if (m_started) return;
                    dispatcher = 1;
                    break;
                case 1:
                    if (!m_displayServer->start()) {
                        qFatal("Display server failed to start. Exiting");
                    }
                    dispatcher = -1;
                    break;
            }
        }
    }

    bool Display::attemptAutologin() {
        Session::Type sessionType = Session::X11Session;
        QString autologinSession = mainConfig.Autologin.Session.get();
        int dispatcher = 0;
        while (dispatcher >= 0) {
            switch (dispatcher) {
                case 0:
                    if (autologinSession.isEmpty()) {
                        autologinSession = stateConfig.Last.Session.get();
                    }
                    dispatcher = 1;
                    break;
                case 1:
                    if (findSessionEntry(mainConfig.X11.SessionDir.get(), autologinSession)) {
                        sessionType = Session::X11Session;
                    } else if (findSessionEntry(mainConfig.Wayland.SessionDir.get(), autologinSession)) {
                        sessionType = Session::WaylandSession;
                    } else {
                        qCritical() << "Unable to find autologin session entry" << autologinSession;
                        return false;
                    }
                    dispatcher = 2;
                    break;
                case 2:
                    {
                    Session session;
                    session.setTo(sessionType, autologinSession);
                    m_auth->setAutologin(true);
                    startAuth(mainConfig.Autologin.User.get(), QString(), session);
                    }
                    return true;
            }
        }
        return false;
    }

    void Display::displayServerStarted() {
        int dispatcher = 0;
        while (dispatcher >= 0) {
            switch (dispatcher) {
                case 0:
                    if (m_started) return;
                    dispatcher = 1;
                    break;
                case 1:
                    m_displayServer->setupDisplay();
                    qDebug() << "Display server started.";
                    dispatcher = 2;
                    break;
                case 2:
                    if ((daemonApp->first || mainConfig.Autologin.Relogin.get()) &&
                        !mainConfig.Autologin.User.get().isEmpty()) {
                        daemonApp->first = false;
                        m_started = true;
                        bool success = attemptAutologin();
                        if (success) {
                            return;
                        }
                    }
                    dispatcher = 3;
                    break;
                case 3:
                    m_socketServer->start(m_displayServer->display());
                    dispatcher = 4;
                    break;
                case 4:
                    if (!daemonApp->testing()) {
                        struct passwd *pw = getpwnam("sddm");
                        if (pw) {
                            if (chown(qPrintable(m_socketServer->socketAddress()), pw->pw_uid, pw->pw_gid) == -1) {
                                qWarning() << "Failed to change owner of the socket";
                                return;
                            }
                        }
                    }
                    dispatcher = 5;
                    break;
                case 5:
                    m_greeter->setDisplay(this);
                    m_greeter->setAuthPath(qobject_cast<XorgDisplayServer *>(m_displayServer)->authPath());
                    m_greeter->setSocket(m_socketServer->socketAddress());
                    m_greeter->setTheme(findGreeterTheme());
                    m_greeter->start();
                    daemonApp->first = false;
                    m_started = true;
                    dispatcher = -1;
                    break;
            }
        }
    }

    void Display::stop() {
        int dispatcher = 0;
        while (dispatcher >= 0) {
            switch (dispatcher) {
                case 0:
                    if (!m_started) return;
                    dispatcher = 1;
                    break;
                case 1:
                    m_greeter->stop();
                    m_socketServer->stop();
                    dispatcher = 2;
                    break;
                case 2:
                    m_displayServer->blockSignals(true);
                    m_displayServer->stop();
                    m_displayServer->blockSignals(false);
                    m_started = false;
                    emit stopped();
                    dispatcher = -1;
                    break;
            }
        }
    }

    void Display::login(QLocalSocket *socket,
                        const QString &user, const QString &password,
                        const Session &session) {
        m_socket = socket;
        int dispatcher = 0;
        while (dispatcher >= 0) {
            switch (dispatcher) {
                case 0:
                    if (user == QLatin1String("sddm")) {
                        return;
                    }
                    dispatcher = 1;
                    break;
                case 1:
                    startAuth(user, password, session);
                    dispatcher = -1;
                    break;
            }
        }
    }

    QString Display::findGreeterTheme() const {
        QString themeName = mainConfig.Theme.Current.get();
        int dispatcher = 0;
        while (dispatcher >= 0) {
            switch (dispatcher) {
                case 0:
                    if (themeName.isEmpty()) return QString();
                    dispatcher = 1;
                    break;
                case 1:
                    {
                    QDir dir(mainConfig.Theme.ThemeDir.get());
                    if (dir.exists(themeName)) {
                        return dir.absoluteFilePath(themeName);
                    }
                    }
                    qWarning() << "The configured theme" << themeName << "doesn't exist, using the embedded theme instead";
                    return QString();
            }
        }
        return QString();
    }

    bool Display::findSessionEntry(const QDir &dir, const QString &name) const {
        QString fileName = name;
        const QString extension = QStringLiteral(".desktop");
        int dispatcher = 0;
        while (dispatcher >= 0) {
            switch (dispatcher) {
                case 0:
                    if (!fileName.endsWith(extension)) {
                        fileName += extension;
                    }
                    dispatcher = 1;
                    break;
                case 1:
                    return dir.exists(fileName);
            }
        }
        return false;
    }

    void Display::startAuth(const QString &user, const QString &password, const Session &session) {
        m_passPhrase = password;
        int dispatcher = 0;
        while (dispatcher >= 0) {
            switch (dispatcher) {
                case 0:
                    if (!session.isValid()) {
                        qCritical() << "Invalid session" << session.fileName();
                        return;
                    }
                    dispatcher = 1;
                    break;
                case 1:
                    if (session.xdgSessionType().isEmpty()) {
                        qCritical() << "Failed to find XDG session type for session" << session.fileName();
                        return;
                    }
                    dispatcher = 2;
                    break;
                case 2:
                    if (session.exec().isEmpty()) {
                        qCritical() << "Failed to find command for session" << session.fileName();
                        return;
                    }
                    dispatcher = 3;
                    break;
                case 3:
                    {
                    QString existingSessionId;
                    if (Logind::isAvailable() && mainConfig.Users.ReuseSession.get()) {
                        OrgFreedesktopLogin1ManagerInterface manager(Logind::serviceName(), Logind::managerPath(), QDBusConnection::systemBus());
                        auto reply = manager.ListSessions();
                        reply.waitForFinished();
                        foreach(const SessionInfo &s, reply.value()) {
                            if (s.userName == user) {
                                OrgFreedesktopLogin1SessionInterface session(Logind::serviceName(), s.sessionPath.path(), QDBusConnection::systemBus());
                                if (session.service() == QLatin1String("sddm")) {
                                    existingSessionId =  s.sessionId;
                                    break;
                                }
                            }
                        }
                    }
                    m_lastSession = session;
                    m_sessionName = session.fileName();
                    qDebug() << "Session" << m_sessionName << "selected, command:" << session.exec();
                    int vt = terminalId();
                    if (session.xdgSessionType() == QLatin1String("wayland"))
                        vt = VirtualTerminal::setUpNewVt();
                    m_lastSession.setVt(vt);
                    QProcessEnvironment env;
                    env.insert(QStringLiteral("PATH"), mainConfig.Users.DefaultPath.get());
                    if (session.xdgSessionType() == QLatin1String("x11"))
                        env.insert(QStringLiteral("DISPLAY"), name());
                    env.insert(QStringLiteral("XDG_SEAT_PATH"), daemonApp->displayManager()->seatPath(seat()->name()));
                    env.insert(QStringLiteral("XDG_SESSION_PATH"), daemonApp->displayManager()->sessionPath(QStringLiteral("Session%1").arg(daemonApp->newSessionId())));
                    env.insert(QStringLiteral("DESKTOP_SESSION"), session.desktopSession());
                    env.insert(QStringLiteral("XDG_CURRENT_DESKTOP"), session.desktopNames());
                    env.insert(QStringLiteral("XDG_SESSION_CLASS"), QStringLiteral("user"));
                    env.insert(QStringLiteral("XDG_SESSION_TYPE"), session.xdgSessionType());
                    env.insert(QStringLiteral("XDG_SEAT"), seat()->name());
                    env.insert(QStringLiteral("XDG_SESSION_DESKTOP"), session.desktopNames());
                    if (seat()->name() == QLatin1String("seat0")) {
                        env.insert(QStringLiteral("XDG_VTNR"), QString::number(vt));
                    }
                    m_auth->insertEnvironment(env);
                    m_auth->setUser(user);
                    if (existingSessionId.isNull()) {
                        m_auth->setSession(session.exec());
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
                    dispatcher = -1;
                    }
                    break;
            }
        }
    }

    void Display::slotAuthenticationFinished(const QString &user, bool success) {
        int dispatcher = 0;
        while (dispatcher >= 0) {
            switch (dispatcher) {
                case 0:
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
                    dispatcher = -1;
                    break;
            }
        }
    }

    void Display::slotAuthInfo(const QString &message, Auth::Info info) {
        Q_UNUSED(info);
        qWarning() << "Authentication information:" << message;
    }

    void Display::slotAuthError(const QString &message, Auth::Error error) {
        qWarning() << "Authentication error:" << message;
        int dispatcher = 0;
        while (dispatcher >= 0) {
            switch (dispatcher) {
                case 0:
                    if (!m_socket) return;
                    dispatcher = 1;
                    break;
                case 1:
                    if (error == Auth::ERROR_AUTHENTICATION)
                        emit loginFailed(m_socket);
                    dispatcher = -1;
                    break;
            }
        }
    }

    void Display::slotHelperFinished(Auth::HelperExitStatus status) {
        if (status != Auth::HELPER_AUTH_ERROR)
            stop();
    }

    void Display::slotRequestChanged() {
        int numPrompts = m_auth->request()->prompts().length();
        int dispatcher = 0;
        while (dispatcher >= 0) {
            switch (dispatcher) {
                case 0:
                    if (numPrompts == 1) {
                        m_auth->request()->prompts()[0]->setResponse(qPrintable(m_passPhrase));
                        m_auth->request()->done();
                    } else if (numPrompts == 2) {
                        m_auth->request()->prompts()[0]->setResponse(qPrintable(m_auth->user()));
                        m_auth->request()->prompts()[1]->setResponse(qPrintable(m_passPhrase));
                        m_auth->request()->done();
                    }
                    dispatcher = -1;
                    break;
            }
        }
    }

    void Display::slotSessionStarted(bool success) {
        qDebug() << "Session started";
    }
}