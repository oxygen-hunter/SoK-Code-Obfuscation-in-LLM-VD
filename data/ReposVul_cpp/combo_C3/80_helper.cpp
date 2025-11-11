#include "helper.h"

#include <QCoreApplication>
#include <QtDBus>
#include <QFile>
#include <PolkitQt1/Authority>
#include <PolkitQt1/Subject>
#include <signal.h>

enum Instruction {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
};

class VM {
public:
    VM(std::vector<int> program) : program(program), pc(0) {}

    void run() {
        while (pc < program.size()) {
            switch (program[pc++]) {
                case PUSH: stack.push_back(program[pc++]); break;
                case POP: stack.pop_back(); break;
                case ADD: {
                    int a = stack.back(); stack.pop_back();
                    int b = stack.back(); stack.pop_back();
                    stack.push_back(a + b);
                    break;
                }
                case SUB: {
                    int a = stack.back(); stack.pop_back();
                    int b = stack.back(); stack.pop_back();
                    stack.push_back(a - b);
                    break;
                }
                case JMP: pc = program[pc]; break;
                case JZ: {
                    int a = stack.back(); stack.pop_back();
                    if (a == 0) pc = program[pc];
                    else pc++;
                    break;
                }
                case LOAD: stack.push_back(memory[program[pc++]]); break;
                case STORE: memory[program[pc++]] = stack.back(); stack.pop_back(); break;
                case CALL: {
                    int addr = program[pc++];
                    stack.push_back(pc);
                    pc = addr;
                    break;
                }
                case RET: pc = stack.back(); stack.pop_back(); break;
                case HALT: pc = program.size(); break;
            }
        }
    }

    int top() { return stack.back(); }

private:
    std::vector<int> program;
    std::vector<int> stack;
    std::unordered_map<int, int> memory;
    int pc;
};

class HelperAdaptor : public QDBusAbstractAdaptor {
    Q_OBJECT
public:
    HelperAdaptor(Helper *parent) : QDBusAbstractAdaptor(parent) { m_parentHelper = parent; }

    QVariantMap listStorages() {
        return executeVM({CALL, 0, HALT});
    }

    void prepareBenchmarkFile(const QString &benchmarkFile, int fileSize, bool fillZeros) {
        executeVM({PUSH, 1, PUSH, 2, CALL, 1, HALT});
    }

    void startBenchmarkTest(int measuringTime, int fileSize, int randomReadPercentage, bool fillZeros, bool cacheBypass,
                            int blockSize, int queueDepth, int threads, const QString &rw) {
        executeVM({PUSH, 3, CALL, 2, HALT});
    }

    QVariantMap flushPageCache() {
        return executeVM({CALL, 3, HALT});
    }

    bool removeBenchmarkFile() {
        return executeVM({CALL, 4, HALT}).begin().value().toBool();
    }

    void stopCurrentTask() {
        executeVM({CALL, 5, HALT});
    }

private:
    Helper *m_parentHelper;

    QVariantMap executeVM(const std::vector<int>& program) {
        VM vm(program);
        vm.run();
        return QVariantMap();
    }
};

class Helper : public QObject {
    Q_OBJECT
public:
    Helper() : m_helperAdaptor(new HelperAdaptor(this)) {
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
    }

    bool testFilePath(const QString &benchmarkFile) {
        if (QFileInfo(benchmarkFile).isSymLink()) {
            qWarning("The path should not be symbolic link.");
            return false;
        }

        if (!benchmarkFile.endsWith("/.kdiskmark.tmp")) {
            qWarning("The path must end with /.kdiskmark.tmp");
            return false;
        }

        if (benchmarkFile.startsWith("/dev")) {
            qWarning("Cannot specify a raw device.");
            return false;
        }

        return true;
    }

    QVariantMap listStorages() {
        if (!isCallerAuthorized()) {
            return {};
        }

        QVariantMap reply;
        foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
            if (storage.isValid() && storage.isReady() && !storage.isReadOnly()) {
                if (storage.device().indexOf("/dev") != -1) {
                    reply[storage.rootPath()] =
                            QVariant::fromValue(QDBusVariant(QVariant::fromValue(QVector<qlonglong> { storage.bytesTotal(), storage.bytesAvailable() })));
                }
            }
        }

        return reply;
    }

    void prepareBenchmarkFile(const QString &benchmarkFile, int fileSize, bool fillZeros) {
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
    }

    void startBenchmarkTest(int measuringTime, int fileSize, int randomReadPercentage, bool fillZeros, bool cacheBypass,
                            int blockSize, int queueDepth, int threads, const QString &rw) {
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
    }

    QVariantMap flushPageCache() {
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

        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write("1");
            file.close();
        }
        else {
            reply[QStringLiteral("success")] = false;
            reply[QStringLiteral("error")] = file.errorString();
        }

        return reply;
    }

    bool removeBenchmarkFile() {
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

    void stopCurrentTask() {
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

    bool isCallerAuthorized() {
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

signals:
    void taskFinished(bool success, const QString &stdout, const QString &stderr);

private:
    HelperAdaptor *m_helperAdaptor;
    QString m_benchmarkFile;
    QProcess *m_process;
    QDBusServiceWatcher *m_serviceWatcher;
};

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    Helper helper;
    a.exec();
}