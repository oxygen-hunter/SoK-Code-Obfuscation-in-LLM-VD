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

        // respond to authentication requests
        m_auth->setVerbose(true);
        connect(m_auth, SIGNAL(requestChanged()), this, SLOT(slotRequestChanged()));
        connect(m_auth, SIGNAL(authentication(QString,bool)), this, SLOT(slotAuthenticationFinished(QString,bool)));
        connect(m_auth, SIGNAL(session(bool)), this, SLOT(slotSessionStarted(bool)));
        connect(m_auth, SIGNAL(finished(Auth::HelperExitStatus)), this, SLOT(slotHelperFinished(Auth::HelperExitStatus)));
        connect(m_auth, SIGNAL(info(QString,Auth::Info)), this, SLOT(slotAuthInfo(QString,Auth::Info)));
        connect(m_auth, SIGNAL(error(QString,Auth::Error)), this, SLOT(slotAuthError(QString,Auth::Error)));

        // restart display after display server ended
        connect(m_displayServer, SIGNAL(started()), this, SLOT(displayServerStarted()));
        connect(m_displayServer, SIGNAL(stopped()), this, SLOT(stop()));

        // connect login signal
        connect(m_socketServer, SIGNAL(login(QLocalSocket*,QString,QString,Session)),
                this, SLOT(login(QLocalSocket*,QString,QString,Session)));

        // connect login result signals
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
        if (getStartedFlag())
            return;

        if (!m_displayServer->start()) {
            qFatal("Display server failed to start. Exiting");
        }
    }

    bool Display::attemptAutologin() {
        Session::Type sessionType = Session::X11Session;

        QString autologinSession = getAutologinSession();
        if (autologinSession.isEmpty()) {
            autologinSession = getLastSession();
        }
        if (findSessionEntry(mainConfig.X11.SessionDir.get(), autologinSession)) {
            sessionType = Session::X11Session;
        } else if (findSessionEntry(mainConfig.Wayland.SessionDir.get(), autologinSession)) {
            sessionType = Session::WaylandSession;
        } else {
            qCritical() << "Unable to find autologin session entry" << autologinSession;
            return false;
        }

        Session session;
        session.setTo(sessionType, autologinSession);

        m_auth->setAutologin(true);
        startAuth(getAutologinUser(), QString(), session);

        return true;
    }

    void Display::displayServerStarted() {
        if (getStartedFlag())
            return;

        m_displayServer->setupDisplay();

        qDebug() << "Display server started.";

        if ((getFirstFlag() || getReloginFlag()) && !getAutologinUser().isEmpty()) {
            setFirstFlag(false);
            setStartedFlag(true);

            bool success = attemptAutologin();
            if (success) {
                return;
            }
        }

        m_socketServer->start(m_displayServer->display());

        if (!getTestingFlag()) {
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

        setFirstFlag(false);
        setStartedFlag(true);
    }

    void Display::stop() {
        if (!getStartedFlag())
            return;

        m_greeter->stop();
        m_socketServer->stop();
        m_displayServer->blockSignals(true);
        m_displayServer->stop();
        m_displayServer->blockSignals(false);
        setStartedFlag(false);
        emit stopped();
    }

    void Display::login(QLocalSocket *socket,
                        const QString &user, const QString &password,
                        const Session &session) {
        m_socket = socket;

        if (user == QLatin1String("sddm")) {
            return;
        }

        startAuth(user, password, session);
    }

    QString Display::findGreeterTheme() const {
        QString themeName = getCurrentTheme();

        if (themeName.isEmpty())
            return QString();

        QDir dir(getThemeDir());

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

    void Display::startAuth(const QString &user, const QString &password, const Session &session) {
        m_passPhrase = password;

        if (!session.isValid()) {
            qCritical() << "Invalid session" << session.fileName();
            return;
        }
        if (session.xdgSessionType().isEmpty()) {
            qCritical() << "Failed to find XDG session type for session" << session.fileName();
            return;
        }
        if (session.exec().isEmpty()) {
            qCritical() << "Failed to find command for session" << session.fileName();
            return;
        }

        QString existingSessionId;

        if (Logind::isAvailable() && getReuseSessionFlag()) {
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
        env.insert(QStringLiteral("PATH"), getDefaultPath());
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
    }

    void Display::slotAuthenticationFinished(const QString &user, bool success) {
        if (success) {
            qDebug() << "Authenticated successfully";

            m_auth->setCookie(qobject_cast<XorgDisplayServer *>(m_displayServer)->cookie());

            if (getRememberLastUserFlag())
                setLastUser(m_auth->user());
            else
                resetLastUser();
            if (getRememberLastSessionFlag())
                setLastSession(m_sessionName);
            else
                resetLastSession();
            saveStateConfig();

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

    bool Display::getStartedFlag() const {
        return m_started;
    }

    void Display::setStartedFlag(bool value) {
        m_started = value;
    }

    bool Display::getFirstFlag() const {
        return daemonApp->first;
    }

    void Display::setFirstFlag(bool value) {
        daemonApp->first = value;
    }

    bool Display::getReloginFlag() const {
        return mainConfig.Autologin.Relogin.get();
    }

    QString Display::getAutologinSession() const {
        return mainConfig.Autologin.Session.get();
    }

    QString Display::getLastSession() const {
        return stateConfig.Last.Session.get();
    }

    QString Display::getAutologinUser() const {
        return mainConfig.Autologin.User.get();
    }

    bool Display::getTestingFlag() const {
        return daemonApp->testing();
    }

    QString Display::getCurrentTheme() const {
        return mainConfig.Theme.Current.get();
    }

    QDir Display::getThemeDir() const {
        return mainConfig.Theme.ThemeDir.get();
    }

    bool Display::getReuseSessionFlag() const {
        return mainConfig.Users.ReuseSession.get();
    }

    QString Display::getDefaultPath() const {
        return mainConfig.Users.DefaultPath.get();
    }

    bool Display::getRememberLastUserFlag() const {
        return mainConfig.Users.RememberLastUser.get();
    }

    void Display::setLastUser(const QString &user) {
        stateConfig.Last.User.set(user);
    }

    void Display::resetLastUser() {
        stateConfig.Last.User.setDefault();
    }

    bool Display::getRememberLastSessionFlag() const {
        return mainConfig.Users.RememberLastSession.get();
    }

    void Display::setLastSession(const QString &sessionName) {
        stateConfig.Last.Session.set(sessionName);
    }

    void Display::resetLastSession() {
        stateConfig.Last.Session.setDefault();
    }

    void Display::saveStateConfig() {
        stateConfig.save();
    }
}