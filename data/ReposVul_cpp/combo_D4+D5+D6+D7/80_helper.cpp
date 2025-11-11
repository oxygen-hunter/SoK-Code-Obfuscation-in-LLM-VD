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
    return m_parentHelper->listStorages();
}

void HelperAdaptor::prepareBenchmarkFile(const QString &benchmarkFile, int fileSize, bool fillZeros)
{
    return m_parentHelper->prepareBenchmarkFile(benchmarkFile, fileSize, fillZeros);
}

void HelperAdaptor::startBenchmarkTest(int measuringTime, int fileSize, int randomReadPercentage, bool fillZeros, bool cacheBypass,
                                       int blockSize, int queueDepth, int threads, const QString &rw)
{
    m_parentHelper->startBenchmarkTest(measuringTime, fileSize, randomReadPercentage, fillZeros, cacheBypass, blockSize, queueDepth, threads, rw);
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
    struct { bool check1; bool check2; } checker = { QDBusConnection::systemBus().isConnected(), 
        QDBusConnection::systemBus().registerService(QStringLiteral("dev.jonmagon.kdiskmark.helperinterface"))};
    
    if (!checker.check1 || !checker.check2 || 
        !QDBusConnection::systemBus().registerObject(QStringLiteral("/Helper"), this)) {
        qWarning() << QDBusConnection::systemBus().lastError().message();
        qApp->quit();
    }

    QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(this);
    serviceWatcher->setConnection(QDBusConnection ::systemBus());
    serviceWatcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);

    connect(serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, qApp, [this, serviceWatcher](const QString &service) {
        serviceWatcher->removeWatchedService(service);
        if (serviceWatcher->watchedServices().isEmpty()) {
            qApp->quit();
        }
    });

    QObject::connect(this, &Helper::taskFinished, m_helperAdaptor, &HelperAdaptor::taskFinished);
}

bool Helper::testFilePath(const QString &benchmarkFile)
{
    struct { bool isLink; bool isEnd; bool isStart; } pathCheck = {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        QFileInfo(benchmarkFile).isSymbolicLink(),
#else
        QFileInfo(benchmarkFile).isSymLink(),
#endif
        benchmarkFile.endsWith("/.kdiskmark.tmp"),
        benchmarkFile.startsWith("/dev")
    };

    if (pathCheck.isLink) {
        qWarning("The path should not be symbolic link.");
        return false;
    }

    if (!pathCheck.isEnd) {
        qWarning("The path must end with /.kdiskmark.tmp");
        return false;
    }

    if (pathCheck.isStart) {
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

    QVariantMap reply;
    foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
        struct { bool valid; bool ready; bool readOnly; } storageCheck = {
            storage.isValid(), storage.isReady(), storage.isReadOnly()
        };
        if (storageCheck.valid && storageCheck.ready && !storageCheck.readOnly) {
            if (storage.device().indexOf("/dev") != -1) {
                reply[storage.rootPath()] =
                        QVariant::fromValue(QDBusVariant(QVariant::fromValue(QVector<qlonglong> { storage.bytesTotal(), storage.bytesAvailable() })));
            }
        }
    }

    return reply;
}

void Helper::prepareBenchmarkFile(const QString &benchmarkFile, int fileSize, bool fillZeros)
{
    if (!isCallerAuthorized()) {
        return;
    }

    if (!m_benchmarkFile.isEmpty()) {
        qWarning() << "The previous benchmarking was not completed correctly.";
        return;
    }

    if (!testFilePath(benchmarkFile)) {
        return;
    }

    m_benchmarkFile = benchmarkFile;

    QProcess *process = new QProcess();
    process->start("fio", QStringList()
                     << QStringLiteral("--output-format=json")
                     << QStringLiteral("--create_only=1")
                     << QStringLiteral("--filename=%1").arg(m_benchmarkFile)
                     << QStringLiteral("--size=%1m").arg(fileSize)
                     << QStringLiteral("--zero_buffers=%1").arg(fillZeros)
                     << QStringLiteral("--name=prepare"));

    connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [=] (int exitCode, QProcess::ExitStatus exitStatus) {
        emit taskFinished(exitStatus == QProcess::NormalExit, QString(process->readAllStandardOutput()), QString(process->readAllStandardError()));
    });
}

void Helper::startBenchmarkTest(int measuringTime, int fileSize, int randomReadPercentage, bool fillZeros, bool cacheBypass,
                                int blockSize, int queueDepth, int threads, const QString &rw)
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

    QProcess *process = new QProcess();
    process->start("fio", QStringList()
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

    connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [=] (int exitCode, QProcess::ExitStatus exitStatus) {
        emit taskFinished(exitStatus == QProcess::NormalExit, QString(process->readAllStandardOutput()), QString(process->readAllStandardError()));
    });
}

QVariantMap Helper::flushPageCache()
{
    if (!isCallerAuthorized()) {
        return {};
    }

    QVariantMap reply;
    reply[QStringLiteral("success")] = true;

    if (!isCallerAuthorized()) {
        reply[QStringLiteral("success")] = false;
        return reply;
    }

    QFile file("/proc/sys/vm/drop_caches");

    struct { bool success; QString error; } fileOp = {false, ""};

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write("1");
        file.close();
        fileOp.success = true;
    }
    else {
        fileOp.error = file.errorString();
    }

    reply[QStringLiteral("success")] = fileOp.success;
    if (!fileOp.success) {
        reply[QStringLiteral("error")] = fileOp.error;
    }

    return reply;
}

bool Helper::removeBenchmarkFile()
{
    if (!isCallerAuthorized()) {
        return false;
    }

    if (m_benchmarkFile.isEmpty()) {
        return false;
    }

    bool deletionState = QFile(m_benchmarkFile).remove();
    m_benchmarkFile.clear();

    return deletionState;
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
        return true;
    default:
        sendErrorReply(QDBusError::AccessDenied);
        if (m_serviceWatcher->watchedServices().isEmpty())
            qApp->quit();
        return false;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Helper helper;
    a.exec();
}