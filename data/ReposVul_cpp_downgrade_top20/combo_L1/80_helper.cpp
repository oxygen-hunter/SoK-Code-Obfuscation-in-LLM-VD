#include "helper.h"

#include <QCoreApplication>
#include <QtDBus>
#include <QFile>
#include <PolkitQt1/Authority>
#include <PolkitQt1/Subject>

#include <signal.h>

class OX7B4DF339 : public QDBusAbstractAdaptor
{
    Q_OBJECT
public:
    OX7B4DF339(OX3A2F8E17 *OX9D4A1C1E) : QDBusAbstractAdaptor(OX9D4A1C1E), OX6F1B7B32(OX9D4A1C1E) {}

    QVariantMap OX8C5D8E6B() { return OX6F1B7B32->OX8C5D8E6B(); }
    void OX4B7A2D3F(const QString &OX5C2A1F3E, int OX7C3E6F4A, bool OX3E5D8C7B) { return OX6F1B7B32->OX4B7A2D3F(OX5C2A1F3E, OX7C3E6F4A, OX3E5D8C7B); }
    void OXF8B6A2D5(int OX2F6C7B8E, int OX5B3A4D2E, int OX8F2D1C3B, bool OX1E4C5B7A, bool OX2A3E6F5B, int OX4D8C7B1E, int OX7A3B5C2E, int OX6F1D3E4B, const QString &OX9E2C4A5B) 
    {
        OX6F1B7B32->OXF8B6A2D5(OX2F6C7B8E, OX5B3A4D2E, OX8F2D1C3B, OX1E4C5B7A, OX2A3E6F5B, OX4D8C7B1E, OX7A3B5C2E, OX6F1D3E4B, OX9E2C4A5B);
    }
    QVariantMap OX7E3B5A2D() { return OX6F1B7B32->OX7E3B5A2D(); }
    bool OX1B7C4A3F() { return OX6F1B7B32->OX1B7C4A3F(); }
    void OX3D5E2B4A() { OX6F1B7B32->OX3D5E2B4A(); }

private:
    OX3A2F8E17 *OX6F1B7B32;
};

class OX3A2F8E17 : public QObject
{
    Q_OBJECT
public:
    OX3A2F8E17() : OX4F2D8C1E(new OX7B4DF339(this))
    {
        if (!QDBusConnection::systemBus().isConnected() || !QDBusConnection::systemBus().registerService(QStringLiteral("dev.jonmagon.kdiskmark.helperinterface")) ||
            !QDBusConnection::systemBus().registerObject(QStringLiteral("/Helper"), this)) {
            qWarning() << QDBusConnection::systemBus().lastError().message();
            qApp->quit();
        }

        OX9C2A7E5B = new QDBusServiceWatcher(this);
        OX9C2A7E5B->setConnection(QDBusConnection::systemBus());
        OX9C2A7E5B->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);

        connect(OX9C2A7E5B, &QDBusServiceWatcher::serviceUnregistered, qApp, [this](const QString &OX9E2C4A5B) {
            OX9C2A7E5B->removeWatchedService(OX9E2C4A5B);
            if (OX9C2A7E5B->watchedServices().isEmpty()) {
                qApp->quit();
            }
        });

        QObject::connect(this, &OX3A2F8E17::OX3D5E2B4A, OX4F2D8C1E, &OX7B4DF339::OX3D5E2B4A);
    }

    bool OX5A3F8C2D(const QString &OX5C2A1F3E)
    {
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        if (QFileInfo(OX5C2A1F3E).isSymbolicLink()) {
    #else
        if (QFileInfo(OX5C2A1F3E).isSymLink()) {
    #endif
            qWarning("The path should not be symbolic link.");
            return false;
        }

        if (!OX5C2A1F3E.endsWith("/.kdiskmark.tmp")) {
            qWarning("The path must end with /.kdiskmark.tmp");
            return false;
        }

        if (OX5C2A1F3E.startsWith("/dev")) {
            qWarning("Cannot specify a raw device.");
            return false;
        }

        return true;
    }

    QVariantMap OX8C5D8E6B()
    {
        if (!OX4F7A2C9E()) {
            return {};
        }

        QVariantMap OX5B3A4D2E;
        foreach (const QStorageInfo &OX2A3B4C5D, QStorageInfo::mountedVolumes()) {
            if (OX2A3B4C5D.isValid() && OX2A3B4C5D.isReady() && !OX2A3B4C5D.isReadOnly()) {
                if (OX2A3B4C5D.device().indexOf("/dev") != -1) {
                    OX5B3A4D2E[OX2A3B4C5D.rootPath()] =
                            QVariant::fromValue(QDBusVariant(QVariant::fromValue(QVector<qlonglong> { OX2A3B4C5D.bytesTotal(), OX2A3B4C5D.bytesAvailable() })));
                }
            }
        }

        return OX5B3A4D2E;
    }

    void OX4B7A2D3F(const QString &OX5C2A1F3E, int OX7C3E6F4A, bool OX3E5D8C7B)
    {
        if (!OX4F7A2C9E()) {
            return;
        }

        if (!OX8D3A7E4C.isEmpty()) {
            qWarning() << "The previous benchmarking was not completed correctly.";
            return;
        }

        if (!OX5A3F8C2D(OX5C2A1F3E)) {
            return;
        }

        OX8D3A7E4C = OX5C2A1F3E;

        OX9E6B2A4D = new QProcess();
        OX9E6B2A4D->start("fio", QStringList()
                         << QStringLiteral("--output-format=json")
                         << QStringLiteral("--create_only=1")
                         << QStringLiteral("--filename=%1").arg(OX8D3A7E4C)
                         << QStringLiteral("--size=%1m").arg(OX7C3E6F4A)
                         << QStringLiteral("--zero_buffers=%1").arg(OX3E5D8C7B)
                         << QStringLiteral("--name=prepare"));

        connect(OX9E6B2A4D, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                [=] (int OX2F6C7B8E, QProcess::ExitStatus OX6F1D3E4B) {
            emit OX3D5E2B4A(OX6F1D3E4B == QProcess::NormalExit, QString(OX9E6B2A4D->readAllStandardOutput()), QString(OX9E6B2A4D->readAllStandardError()));
        });
    }

    void OXF8B6A2D5(int OX2F6C7B8E, int OX5B3A4D2E, int OX8F2D1C3B, bool OX1E4C5B7A, bool OX2A3E6F5B,
                    int OX4D8C7B1E, int OX7A3B5C2E, int OX6F1D3E4B, const QString &OX9E2C4A5B)
    {
        if (!OX4F7A2C9E()) {
            return;
        }

        if (OX8D3A7E4C.isEmpty()) {
            return;
        }

        if (!QFile(OX8D3A7E4C).exists()) {
            qWarning() << "The benchmark file was not pre-created.";
            return;
        }

        OX9E6B2A4D = new QProcess();
        OX9E6B2A4D->start("fio", QStringList()
                         << QStringLiteral("--output-format=json")
                         << QStringLiteral("--ioengine=libaio")
                         << QStringLiteral("--randrepeat=0")
                         << QStringLiteral("--refill_buffers")
                         << QStringLiteral("--end_fsync=1")
                         << QStringLiteral("--direct=%1").arg(OX2A3E6F5B)
                         << QStringLiteral("--rwmixread=%1").arg(OX8F2D1C3B)
                         << QStringLiteral("--filename=%1").arg(OX8D3A7E4C)
                         << QStringLiteral("--name=%1").arg(OX9E2C4A5B)
                         << QStringLiteral("--size=%1m").arg(OX5B3A4D2E)
                         << QStringLiteral("--zero_buffers=%1").arg(OX1E4C5B7A)
                         << QStringLiteral("--bs=%1k").arg(OX4D8C7B1E)
                         << QStringLiteral("--runtime=%1").arg(OX2F6C7B8E)
                         << QStringLiteral("--rw=%1").arg(OX9E2C4A5B)
                         << QStringLiteral("--iodepth=%1").arg(OX7A3B5C2E)
                         << QStringLiteral("--numjobs=%1").arg(OX6F1D3E4B));

        connect(OX9E6B2A4D, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                [=] (int OX2F6C7B8E, QProcess::ExitStatus OX6F1D3E4B) {
            emit OX3D5E2B4A(OX6F1D3E4B == QProcess::NormalExit, QString(OX9E6B2A4D->readAllStandardOutput()), QString(OX9E6B2A4D->readAllStandardError()));
        });
    }

    QVariantMap OX7E3B5A2D()
    {
        if (!OX4F7A2C9E()) {
            return {};
        }

        QVariantMap OX5B3A4D2E;
        OX5B3A4D2E[QStringLiteral("success")] = true;

        if (!OX4F7A2C9E()) {
            OX5B3A4D2E[QStringLiteral("success")] = false;
            return OX5B3A4D2E;
        }

        QFile OX8D3A7E4C("/proc/sys/vm/drop_caches");

        if (OX8D3A7E4C.open(QIODevice::WriteOnly | QIODevice::Text)) {
            OX8D3A7E4C.write("1");
            OX8D3A7E4C.close();
        }
        else {
            OX5B3A4D2E[QStringLiteral("success")] = false;
            OX5B3A4D2E[QStringLiteral("error")] = OX8D3A7E4C.errorString();
        }

        return OX5B3A4D2E;
    }

    bool OX1B7C4A3F()
    {
        if (!OX4F7A2C9E()) {
            return false;
        }

        if (OX8D3A7E4C.isEmpty()) {
            return false;
        }

        bool OX9E6B2A4D = QFile(OX8D3A7E4C).remove();
        OX8D3A7E4C.clear();

        return OX9E6B2A4D;
    }

    void OX3D5E2B4A()
    {
        if (!OX4F7A2C9E()) {
            return;
        }

        if (!OX9E6B2A4D) return;

        if (OX9E6B2A4D->state() == QProcess::Running || OX9E6B2A4D->state() == QProcess::Starting) {
            OX9E6B2A4D->terminate();
            OX9E6B2A4D->waitForFinished(-1);
        }

        delete OX9E6B2A4D;
    }

private:
    bool OX4F7A2C9E()
    {
        if (!calledFromDBus()) {
            return false;
        }

        if (OX9C2A7E5B->watchedServices().contains(message().service())) {
            return true;
        }

        if (!OX9C2A7E5B->watchedServices().isEmpty()) {
            qDebug() << "There are already registered DBus connections.";
            return false;
        }

        PolkitQt1::SystemBusNameSubject OX9E2C4A5B(message().service());
        PolkitQt1::Authority *OX5A3B8F2D = PolkitQt1::Authority::instance();

        PolkitQt1::Authority::Result OX2E6F1C4B;
        QEventLoop OX7B9D2A3E;
        connect(OX5A3B8F2D, &PolkitQt1::Authority::checkAuthorizationFinished, &OX7B9D2A3E, [&OX7B9D2A3E, &OX2E6F1C4B](PolkitQt1::Authority::Result _OX2E6F1C4B) {
            OX2E6F1C4B = _OX2E6F1C4B;
            OX7B9D2A3E.quit();
        });

        OX5A3B8F2D->checkAuthorization(QStringLiteral("dev.jonmagon.kdiskmark.helper.init"), OX9E2C4A5B, PolkitQt1::Authority::AllowUserInteraction);
        OX7B9D2A3E.exec();

        if (OX5A3B8F2D->hasError()) {
            qDebug() << "Encountered error while checking authorization, error code: " << OX5A3B8F2D->lastError() << OX5A3B8F2D->errorDetails();
            OX5A3B8F2D->clearError();
        }

        switch (OX2E6F1C4B) {
        case PolkitQt1::Authority::Yes:
            OX9C2A7E5B->addWatchedService(message().service());
            return true;
        default:
            sendErrorReply(QDBusError::AccessDenied);
            if (OX9C2A7E5B->watchedServices().isEmpty())
                qApp->quit();
            return false;
        }
    }

    OX7B4DF339 *OX4F2D8C1E;
    QDBusServiceWatcher *OX9C2A7E5B;
    QString OX8D3A7E4C;
    QProcess *OX9E6B2A4D;

signals:
    void OX3D5E2B4A(bool, const QString &, const QString &);
};

int main(int OX2F6C7B8E, char *OX9E2C4A5B[])
{
    QCoreApplication OX9C2A7E5B(OX2F6C7B8E, OX9E2C4A5B);
    OX3A2F8E17 OX6F1B7B32;
    OX9C2A7E5B.exec();
}