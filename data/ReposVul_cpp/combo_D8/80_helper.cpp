#include "helper.h"

#include <QCoreApplication>
#include <QtDBus>
#include <QFile>
#include <PolkitQt1/Authority>
#include <PolkitQt1/Subject>

#include <signal.h>

HelperAdaptor::HelperAdaptor(Helper *p) :
    QDBusAbstractAdaptor(p)
{
    m_parentHelper = p;
}

QVariantMap HelperAdaptor::listStorages()
{
    return m_parentHelper->listStorages();
}

void HelperAdaptor::prepareBenchmarkFile(const QString &b, int s, bool f)
{
    return m_parentHelper->prepareBenchmarkFile(b, s, f);
}

void HelperAdaptor::startBenchmarkTest(int t, int s, int r, bool f, bool c, int b, int q, int th, const QString &rw)
{
    m_parentHelper->startBenchmarkTest(t, s, r, f, c, b, q, th, rw);
}

QVariantMap HelperAdaptor::flushPageCache()
{
    return m_parentHelper->flushPageCache();
}

bool HelperAdaptor::removeBenchmarkFile()
{
    return m_parentHelper->removeBenchmarkFile();
}

void HelperAdaptor::stopCurrentTask()
{
    m_parentHelper->stopCurrentTask();
}

Helper::Helper() : m_helperAdaptor(new HelperAdaptor(this))
{
    if (!QDBusConnection::systemBus().isConnected() || !QDBusConnection::systemBus().registerService(QStringLiteral("dev.jonmagon.kdiskmark.helperinterface")) ||
        !QDBusConnection::systemBus().registerObject(QStringLiteral("/Helper"), this)) {
        qWarning() << QDBusConnection::systemBus().lastError().message();
        qApp->quit();
    }

    m_serviceWatcher = new QDBusServiceWatcher(this);
    m_serviceWatcher->setConnection(QDBusConnection ::systemBus());
    m_serviceWatcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);

    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, qApp, [this](const QString &s) {
        m_serviceWatcher->removeWatchedService(s);
        if (m_serviceWatcher->watchedServices().isEmpty()) {
            qApp->quit();
        }
    });

    QObject::connect(this, &Helper::taskFinished, m_helperAdaptor, &HelperAdaptor::taskFinished);
}

bool Helper::testFilePath(const QString &b)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    if (QFileInfo(b).isSymbolicLink()) {
#else
    if (QFileInfo(b).isSymLink()) {
#endif
        qWarning("The path should not be symbolic link.");
        return false;
    }

    if (!b.endsWith("/.kdiskmark.tmp")) {
        qWarning("The path must end with /.kdiskmark.tmp");
        return false;
    }

    if (b.startsWith("/dev")) {
        qWarning("Cannot specify a raw device.");
        return false;
    }

    return true;
}

QVariantMap Helper::listStorages()
{
    if (!isCallerAuthorized()) {
        return {};
    }

    QVariantMap r;
    foreach (const QStorageInfo &s, QStorageInfo::mountedVolumes()) {
        if (s.isValid() && s.isReady() && !s.isReadOnly()) {
            if (s.device().indexOf("/dev") != -1) {
                r[s.rootPath()] =
                        QVariant::fromValue(QDBusVariant(QVariant::fromValue(QVector<qlonglong> { s.bytesTotal(), s.bytesAvailable() })));
            }
        }
    }

    return r;
}

void Helper::prepareBenchmarkFile(const QString &b, int s, bool f)
{
    if (!isCallerAuthorized()) {
        return;
    }

    if (!m_benchmarkFile.isEmpty()) {
        qWarning() << "The previous benchmarking was not completed correctly.";
        return;
    }

    if (!testFilePath(b)) {
        return;
    }

    m_benchmarkFile = b;

    m_process = new QProcess();
    m_process->start("fio", QStringList()
                     << QStringLiteral("--output-format=json")
                     << QStringLiteral("--create_only=1")
                     << QStringLiteral("--filename=%1").arg(m_benchmarkFile)
                     << QStringLiteral("--size=%1m").arg(s)
                     << QStringLiteral("--zero_buffers=%1").arg(f)
                     << QStringLiteral("--name=prepare"));

    connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [=] (int e, QProcess::ExitStatus es) {
        emit taskFinished(es == QProcess::NormalExit, QString(m_process->readAllStandardOutput()), QString(m_process->readAllStandardError()));
    });
}

void Helper::startBenchmarkTest(int t, int s, int r, bool f, bool c, int b, int q, int th, const QString &rw)
{
    if (!isCallerAuthorized()) {
        return;
    }

    if (m_benchmarkFile.isEmpty()) {
        return;
    }

    if (!QFile(m_benchmarkFile).exists()) {
        qWarning() << "The benchmark file was not pre-created.";
        return;
    }

    m_process = new QProcess();
    m_process->start("fio", QStringList()
                     << QStringLiteral("--output-format=json")
                     << QStringLiteral("--ioengine=libaio")
                     << QStringLiteral("--randrepeat=0")
                     << QStringLiteral("--refill_buffers")
                     << QStringLiteral("--end_fsync=1")
                     << QStringLiteral("--direct=%1").arg(c)
                     << QStringLiteral("--rwmixread=%1").arg(r)
                     << QStringLiteral("--filename=%1").arg(m_benchmarkFile)
                     << QStringLiteral("--name=%1").arg(rw)
                     << QStringLiteral("--size=%1m").arg(s)
                     << QStringLiteral("--zero_buffers=%1").arg(f)
                     << QStringLiteral("--bs=%1k").arg(b)
                     << QStringLiteral("--runtime=%1").arg(t)
                     << QStringLiteral("--rw=%1").arg(rw)
                     << QStringLiteral("--iodepth=%1").arg(q)
                     << QStringLiteral("--numjobs=%1").arg(th));

    connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [=] (int e, QProcess::ExitStatus es) {
        emit taskFinished(es == QProcess::NormalExit, QString(m_process->readAllStandardOutput()), QString(m_process->readAllStandardError()));
    });
}

QVariantMap Helper::flushPageCache()
{
    if (!isCallerAuthorized()) {
        return {};
    }

    QVariantMap r;
    r[QStringLiteral("success")] = true;

    if (!isCallerAuthorized()) {
        r[QStringLiteral("success")] = false;
        return r;
    }

    QFile f("/proc/sys/vm/drop_caches");

    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        f.write("1");
        f.close();
    }
    else {
        r[QStringLiteral("success")] = false;
        r[QStringLiteral("error")] = f.errorString();
    }

    return r;
}

bool Helper::removeBenchmarkFile()
{
    if (!isCallerAuthorized()) {
        return false;
    }

    if (m_benchmarkFile.isEmpty()) {
        return false;
    }

    bool d = QFile(m_benchmarkFile).remove();
    m_benchmarkFile.clear();

    return d;
}

void Helper::stopCurrentTask()
{
    if (!isCallerAuthorized()) {
        return;
    }

    if (!m_process) return;

    if (m_process->state() == QProcess::Running || m_process->state() == QProcess::Starting) {
        m_process->terminate();
        m_process->waitForFinished(-1);
    }

    delete m_process;
}

bool Helper::isCallerAuthorized()
{
    if (!calledFromDBus()) {
        return false;
    }

    if (m_serviceWatcher->watchedServices().contains(message().service())) {
        return true;
    }

    if (!m_serviceWatcher->watchedServices().isEmpty()) {
        qDebug() << "There are already registered DBus connections.";
        return false;
    }

    PolkitQt1::SystemBusNameSubject subj(message().service());
    PolkitQt1::Authority *auth = PolkitQt1::Authority::instance();

    PolkitQt1::Authority::Result res;
    QEventLoop e;
    connect(auth, &PolkitQt1::Authority::checkAuthorizationFinished, &e, [&e, &res](PolkitQt1::Authority::Result r) {
        res = r;
        e.quit();
    });

    auth->checkAuthorization(QStringLiteral("dev.jonmagon.kdiskmark.helper.init"), subj, PolkitQt1::Authority::AllowUserInteraction);
    e.exec();

    if (auth->hasError()) {
        qDebug() << "Encountered error while checking authorization, error code: " << auth->lastError() << auth->errorDetails();
        auth->clearError();
    }

    switch (res) {
    case PolkitQt1::Authority::Yes:
        m_serviceWatcher->addWatchedService(message().service());
        return true;
    default:
        sendErrorReply(QDBusError::AccessDenied);
        if (m_serviceWatcher->watchedServices().isEmpty())
            qApp->quit();
        return false;
    }
}

int main(int c, char *v[])
{
    QCoreApplication a(c, v);
    Helper h;
    a.exec();
}