#include "helper.h"

#include <QCoreApplication>
#include <QtDBus>
#include <QFile>
#include <PolkitQt1/Authority>
#include <PolkitQt1/Subject>
#include <signal.h>

HelperAdaptor::HelperAdaptor(Helper *parent) :
    QDBusAbstractAdaptor(parent)
{
    m_parentHelper = parent;
}

QVariantMap HelperAdaptor::listStorages()
{
    QVariantMap result;
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                result = m_parentHelper->listStorages();
                state = 1;
                break;
            case 1:
                return result;
        }
    }
}

void HelperAdaptor::prepareBenchmarkFile(const QString &benchmarkFile, int fileSize, bool fillZeros)
{
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                m_parentHelper->prepareBenchmarkFile(benchmarkFile, fileSize, fillZeros);
                state = 1;
                break;
            case 1:
                return;
        }
    }
}

void HelperAdaptor::startBenchmarkTest(int measuringTime, int fileSize, int randomReadPercentage, bool fillZeros, bool cacheBypass,
                                       int blockSize, int queueDepth, int threads, const QString &rw)
{
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                m_parentHelper->startBenchmarkTest(measuringTime, fileSize, randomReadPercentage, fillZeros, cacheBypass, blockSize, queueDepth, threads, rw);
                state = 1;
                break;
            case 1:
                return;
        }
    }
}

QVariantMap HelperAdaptor::flushPageCache()
{
    QVariantMap result;
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                result = m_parentHelper->flushPageCache();
                state = 1;
                break;
            case 1:
                return result;
        }
    }
}

bool HelperAdaptor::removeBenchmarkFile()
{
    bool result;
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                result = m_parentHelper->removeBenchmarkFile();
                state = 1;
                break;
            case 1:
                return result;
        }
    }
}

void HelperAdaptor::stopCurrentTask()
{
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                m_parentHelper->stopCurrentTask();
                state = 1;
                break;
            case 1:
                return;
        }
    }
}

Helper::Helper() : m_helperAdaptor(new HelperAdaptor(this))
{
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (!QDBusConnection::systemBus().isConnected() || !QDBusConnection::systemBus().registerService(QStringLiteral("dev.jonmagon.kdiskmark.helperinterface")) ||
                    !QDBusConnection::systemBus().registerObject(QStringLiteral("/Helper"), this)) {
                    qWarning() << QDBusConnection::systemBus().lastError().message();
                    qApp->quit();
                }

                m_serviceWatcher = new QDBusServiceWatcher(this);
                m_serviceWatcher->setConnection(QDBusConnection::systemBus());
                m_serviceWatcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);

                connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, qApp, [this](const QString &service) {
                    m_serviceWatcher->removeWatchedService(service);
                    if (m_serviceWatcher->watchedServices().isEmpty()) {
                        qApp->quit();
                    }
                });

                QObject::connect(this, &Helper::taskFinished, m_helperAdaptor, &HelperAdaptor::taskFinished);
                state = 1;
                break;
            case 1:
                return;
        }
    }
}

bool Helper::testFilePath(const QString &benchmarkFile)
{
    bool result = false;
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
                if (QFileInfo(benchmarkFile).isSymbolicLink()) {
                #else
                if (QFileInfo(benchmarkFile).isSymLink()) {
                #endif
                    qWarning("The path should not be symbolic link.");
                    state = 1;
                    break;
                }
                state = 2;
                break;
            case 1:
                result = false;
                state = 9;
                break;
            case 2:
                if (!benchmarkFile.endsWith("/.kdiskmark.tmp")) {
                    qWarning("The path must end with /.kdiskmark.tmp");
                    state = 3;
                    break;
                }
                state = 4;
                break;
            case 3:
                result = false;
                state = 9;
                break;
            case 4:
                if (benchmarkFile.startsWith("/dev")) {
                    qWarning("Cannot specify a raw device.");
                    state = 5;
                    break;
                }
                state = 6;
                break;
            case 5:
                result = false;
                state = 9;
                break;
            case 6:
                result = true;
                state = 9;
                break;
            case 9:
                return result;
        }
    }
}

QVariantMap Helper::listStorages()
{
    QVariantMap reply;
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (!isCallerAuthorized()) {
                    state = 1;
                    break;
                }
                state = 2;
                break;
            case 1:
                return reply;
            case 2:
                foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
                    if (storage.isValid() && storage.isReady() && !storage.isReadOnly()) {
                        if (storage.device().indexOf("/dev") != -1) {
                            reply[storage.rootPath()] =
                                    QVariant::fromValue(QDBusVariant(QVariant::fromValue(QVector<qlonglong> { storage.bytesTotal(), storage.bytesAvailable() })));
                        }
                    }
                }
                state = 3;
                break;
            case 3:
                return reply;
        }
    }
}

void Helper::prepareBenchmarkFile(const QString &benchmarkFile, int fileSize, bool fillZeros)
{
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (!isCallerAuthorized()) {
                    state = 1;
                    break;
                }
                state = 2;
                break;
            case 1:
                return;
            case 2:
                if (!m_benchmarkFile.isEmpty()) {
                    qWarning() << "The previous benchmarking was not completed correctly.";
                    state = 3;
                    break;
                }
                if (!testFilePath(benchmarkFile)) {
                    state = 3;
                    break;
                }
                state = 4;
                break;
            case 3:
                return;
            case 4:
                m_benchmarkFile = benchmarkFile;
                m_process = new QProcess();
                m_process->start("fio", QStringList()
                                 << QStringLiteral("--output-format=json")
                                 << QStringLiteral("--create_only=1")
                                 << QStringLiteral("--filename=%1").arg(m_benchmarkFile)
                                 << QStringLiteral("--size=%1m").arg(fileSize)
                                 << QStringLiteral("--zero_buffers=%1").arg(fillZeros)
                                 << QStringLiteral("--name=prepare"));
                connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                        [=] (int exitCode, QProcess::ExitStatus exitStatus) {
                    emit taskFinished(exitStatus == QProcess::NormalExit, QString(m_process->readAllStandardOutput()), QString(m_process->readAllStandardError()));
                });
                state = 5;
                break;
            case 5:
                return;
        }
    }
}

void Helper::startBenchmarkTest(int measuringTime, int fileSize, int randomReadPercentage, bool fillZeros, bool cacheBypass,
                                int blockSize, int queueDepth, int threads, const QString &rw)
{
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (!isCallerAuthorized()) {
                    state = 1;
                    break;
                }
                if (m_benchmarkFile.isEmpty()) {
                    state = 1;
                    break;
                }
                if (!QFile(m_benchmarkFile).exists()) {
                    qWarning() << "The benchmark file was not pre-created.";
                    state = 1;
                    break;
                }
                state = 2;
                break;
            case 1:
                return;
            case 2:
                m_process = new QProcess();
                m_process->start("fio", QStringList()
                                 << QStringLiteral("--output-format=json")
                                 << QStringLiteral("--ioengine=libaio")
                                 << QStringLiteral("--randrepeat=0")
                                 << QStringLiteral("--refill_buffers")
                                 << QStringLiteral("--end_fsync=1")
                                 << QStringLiteral("--direct=%1").arg(cacheBypass)
                                 << QStringLiteral("--rwmixread=%1").arg(randomReadPercentage)
                                 << QStringLiteral("--filename=%1").arg(m_benchmarkFile)
                                 << QStringLiteral("--name=%1").arg(rw)
                                 << QStringLiteral("--size=%1m").arg(fileSize)
                                 << QStringLiteral("--zero_buffers=%1").arg(fillZeros)
                                 << QStringLiteral("--bs=%1k").arg(blockSize)
                                 << QStringLiteral("--runtime=%1").arg(measuringTime)
                                 << QStringLiteral("--rw=%1").arg(rw)
                                 << QStringLiteral("--iodepth=%1").arg(queueDepth)
                                 << QStringLiteral("--numjobs=%1").arg(threads));
                connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                        [=] (int exitCode, QProcess::ExitStatus exitStatus) {
                    emit taskFinished(exitStatus == QProcess::NormalExit, QString(m_process->readAllStandardOutput()), QString(m_process->readAllStandardError()));
                });
                state = 3;
                break;
            case 3:
                return;
        }
    }
}

QVariantMap Helper::flushPageCache()
{
    QVariantMap reply;
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (!isCallerAuthorized()) {
                    state = 1;
                    break;
                }
                reply[QStringLiteral("success")] = true;
                state = 2;
                break;
            case 1:
                return reply;
            case 2:
                if (!isCallerAuthorized()) {
                    reply[QStringLiteral("success")] = false;
                    state = 3;
                    break;
                }
                state = 4;
                break;
            case 3:
                return reply;
            case 4:
                QFile file("/proc/sys/vm/drop_caches");
                if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    file.write("1");
                    file.close();
                } else {
                    reply[QStringLiteral("success")] = false;
                    reply[QStringLiteral("error")] = file.errorString();
                }
                state = 5;
                break;
            case 5:
                return reply;
        }
    }
}

bool Helper::removeBenchmarkFile()
{
    bool result = false;
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (!isCallerAuthorized()) {
                    state = 1;
                    break;
                }
                if (m_benchmarkFile.isEmpty()) {
                    state = 1;
                    break;
                }
                bool deletionState = QFile(m_benchmarkFile).remove();
                m_benchmarkFile.clear();
                result = deletionState;
                state = 2;
                break;
            case 1:
                result = false;
                state = 2;
                break;
            case 2:
                return result;
        }
    }
}

void Helper::stopCurrentTask()
{
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (!isCallerAuthorized()) {
                    state = 1;
                    break;
                }
                if (!m_process) {
                    state = 1;
                    break;
                }
                if (m_process->state() == QProcess::Running || m_process->state() == QProcess::Starting) {
                    m_process->terminate();
                    m_process->waitForFinished(-1);
                }
                delete m_process;
                state = 2;
                break;
            case 1:
                return;
            case 2:
                return;
        }
    }
}

bool Helper::isCallerAuthorized()
{
    bool result = false;
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (!calledFromDBus()) {
                    state = 1;
                    break;
                }
                if (m_serviceWatcher->watchedServices().contains(message().service())) {
                    result = true;
                    state = 2;
                    break;
                }
                if (!m_serviceWatcher->watchedServices().isEmpty()) {
                    qDebug() << "There are already registered DBus connections.";
                    state = 1;
                    break;
                }
                state = 3;
                break;
            case 1:
                result = false;
                state = 2;
                break;
            case 2:
                return result;
            case 3:
                PolkitQt1::SystemBusNameSubject subject(message().service());
                PolkitQt1::Authority *authority = PolkitQt1::Authority::instance();

                PolkitQt1::Authority::Result result;
                QEventLoop e;
                connect(authority, &PolkitQt1::Authority::checkAuthorizationFinished, &e, [&e, &result](PolkitQt1::Authority::Result _result) {
                    result = _result;
                    e.quit();
                });

                authority->checkAuthorization(QStringLiteral("dev.jonmagon.kdiskmark.helper.init"), subject, PolkitQt1::Authority::AllowUserInteraction);
                e.exec();

                if (authority->hasError()) {
                    qDebug() << "Encountered error while checking authorization, error code: " << authority->lastError() << authority->errorDetails();
                    authority->clearError();
                }
                switch (result) {
                    case PolkitQt1::Authority::Yes:
                        m_serviceWatcher->addWatchedService(message().service());
                        result = true;
                        state = 4;
                        break;
                    default:
                        sendErrorReply(QDBusError::AccessDenied);
                        if (m_serviceWatcher->watchedServices().isEmpty())
                            qApp->quit();
                        result = false;
                        state = 4;
                        break;
                }
                break;
            case 4:
                return result;
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Helper helper;
    a.exec();
}