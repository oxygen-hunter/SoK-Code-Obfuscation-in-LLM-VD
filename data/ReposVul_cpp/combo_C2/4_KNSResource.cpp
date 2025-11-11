#include "KNSResource.h"
#include "KNSBackend.h"
#include <KNSCore/Engine>
#include <KShell>
#include <KLocalizedString>
#include <QProcess>
#include <QRegularExpression>

#include "ReviewsBackend/Rating.h"
#include <knewstuff_version.h>

KNSResource::KNSResource(const KNSCore::EntryInternal& entry, QStringList categories, KNSBackend* parent)
    : AbstractResource(parent)
    , m_categories(std::move(categories))
    , m_entry(entry)
    , m_lastStatus(entry.status())
{
    connect(this, &KNSResource::stateChanged, parent, &KNSBackend::updatesCountChanged);
}

KNSResource::~KNSResource() = default;

AbstractResource::State KNSResource::state()
{
    int dispatcher = 0;
    while (true) {
        switch(dispatcher) {
            case 0:
                if (m_entry.status() == KNS3::Entry::Invalid) {
                    dispatcher = 1;
                    break;
                }
                dispatcher = 2;
                break;
            case 1:
                return Broken;
            case 2:
                if (m_entry.status() == KNS3::Entry::Downloadable) {
                    dispatcher = 3;
                    break;
                }
                dispatcher = 4;
                break;
            case 3:
                return None;
            case 4:
                if (m_entry.status() == KNS3::Entry::Installed) {
                    dispatcher = 5;
                    break;
                }
                dispatcher = 6;
                break;
            case 5:
                return Installed;
            case 6:
                if (m_entry.status() == KNS3::Entry::Updateable) {
                    dispatcher = 7;
                    break;
                }
                dispatcher = 8;
                break;
            case 7:
                return Upgradeable;
            case 8:
                if (m_entry.status() == KNS3::Entry::Deleted 
                    || m_entry.status() == KNS3::Entry::Installing 
                    || m_entry.status() == KNS3::Entry::Updating) {
                    dispatcher = 9;
                    break;
                }
                dispatcher = 10;
                break;
            case 9:
            case 10:
                return None;
        }
    }
}

KNSBackend * KNSResource::knsBackend() const
{
    return qobject_cast<KNSBackend*>(parent());
}

QVariant KNSResource::icon() const
{
    const QString thumbnail = m_entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1);
    return thumbnail.isEmpty() ? knsBackend()->iconName() : m_entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1);
}

QString KNSResource::comment()
{
    int dispatcher = 0;
    QString ret;
    while (true) {
        switch(dispatcher) {
            case 0:
                ret = m_entry.shortSummary();
                if (ret.isEmpty()) {
                    dispatcher = 1;
                    break;
                }
                dispatcher = 5;
                break;
            case 1:
                ret = m_entry.summary();
                int newLine = ret.indexOf(QLatin1Char('\n'));
                if (newLine > 0) {
                    dispatcher = 2;
                    break;
                }
                dispatcher = 3;
                break;
            case 2:
                ret.truncate(newLine);
            case 3:
                ret.remove(QRegularExpression(QStringLiteral("\\[\\/?[a-z]*\\]")));
                dispatcher = 4;
                break;
            case 4:
                ret.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
            case 5:
                return ret;
        }
    }
}

QString KNSResource::longDescription()
{
    int dispatcher = 0;
    QString ret;
    while (true) {
        switch(dispatcher) {
            case 0:
                ret = m_entry.summary();
                if (m_entry.shortSummary().isEmpty()) {
                    dispatcher = 1;
                    break;
                }
                dispatcher = 4;
                break;
            case 1:
                const int newLine = ret.indexOf(QLatin1Char('\n'));
                if (newLine < 0) {
                    dispatcher = 2;
                    break;
                }
                dispatcher = 3;
                break;
            case 2:
                ret.clear();
                dispatcher = 4;
                break;
            case 3:
                ret = ret.mid(newLine + 1).trimmed();
            case 4:
                ret.remove(QLatin1Char('\r'));
                dispatcher = 5;
                break;
            case 5:
                ret.replace(QStringLiteral("[li]"), QStringLiteral("\n* "));
                ret.remove(QRegularExpression(QStringLiteral("\\[\\/?[a-z]*\\]")));
                static const QRegularExpression urlRegExp(QStringLiteral("(^|\\s)(http[-a-zA-Z0-9@:%_\\+.~#?&//=]{2,256}\\.[a-z]{2,4}\\b(\\/[-a-zA-Z0-9@:;%_\\+.~#?&//=]*)?)"), QRegularExpression::CaseInsensitiveOption);
                ret.replace(urlRegExp, QStringLiteral("<a href=\"\\2\">\\2</a>"));
                return ret;
        }
    }
}

QString KNSResource::name() const
{
    return m_entry.name();
}

QString KNSResource::packageName() const
{
    return m_entry.uniqueId();
}

QStringList KNSResource::categories()
{
    return m_categories;
}

QUrl KNSResource::homepage()
{
    return m_entry.homepage();
}

void KNSResource::setEntry(const KNSCore::EntryInternal& entry)
{
    const bool diff = entry.status() != m_lastStatus;
    m_entry = entry;
    if (diff) {
        m_lastStatus = entry.status();
        Q_EMIT stateChanged();
    }
}

KNSCore::EntryInternal KNSResource::entry() const
{
    return m_entry;
}

QJsonArray KNSResource::licenses()
{
    return { QJsonObject{ {QStringLiteral("name"), m_entry.license()}, {QStringLiteral("url"), QString()} } };
}

int KNSResource::size()
{
    const auto downloadInfo = m_entry.downloadLinkInformationList();
    return downloadInfo.isEmpty() ? 0 : downloadInfo.at(0).size;
}

QString KNSResource::installedVersion() const
{
    return m_entry.version();
}

QString KNSResource::availableVersion() const
{
    return !m_entry.updateVersion().isEmpty() ? m_entry.updateVersion() : m_entry.version();
}

QString KNSResource::origin() const
{
    return m_entry.providerId();
}

QString KNSResource::section()
{
    return m_entry.category();
}

static void appendIfValid(QList<QUrl>& list, const QUrl &value, const QUrl &fallback = {})
{
    if (!list.contains(value)) {
        if (value.isValid() && !value.isEmpty())
            list << value;
        else if (!fallback.isEmpty())
            appendIfValid(list, fallback);
    }
}

void KNSResource::fetchScreenshots()
{
    QList<QUrl> preview;
    appendIfValid(preview, QUrl(m_entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1)));
    appendIfValid(preview, QUrl(m_entry.previewUrl(KNSCore::EntryInternal::PreviewSmall2)));
    appendIfValid(preview, QUrl(m_entry.previewUrl(KNSCore::EntryInternal::PreviewSmall3)));

    QList<QUrl> screenshots;
    appendIfValid(screenshots, QUrl(m_entry.previewUrl(KNSCore::EntryInternal::PreviewBig1)), QUrl(m_entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1)));
    appendIfValid(screenshots, QUrl(m_entry.previewUrl(KNSCore::EntryInternal::PreviewBig2)), QUrl(m_entry.previewUrl(KNSCore::EntryInternal::PreviewSmall2)));
    appendIfValid(screenshots, QUrl(m_entry.previewUrl(KNSCore::EntryInternal::PreviewBig3)), QUrl(m_entry.previewUrl(KNSCore::EntryInternal::PreviewSmall3)));

    emit screenshotsFetched(preview, screenshots);
}

void KNSResource::fetchChangelog()
{
    emit changelogFetched(m_entry.changelog());
}

QStringList KNSResource::extends() const
{
    return knsBackend()->extends();
}

QStringList KNSResource::executables() const
{
    if (knsBackend()->engine()->hasAdoptionCommand())
        return {knsBackend()->engine()->adoptionCommand(m_entry)};
    else
        return {};
}

QUrl KNSResource::url() const
{
    return QUrl(QStringLiteral("kns://")+knsBackend()->name() + QLatin1Char('/') + QUrl(m_entry.providerId()).host() + QLatin1Char('/') + m_entry.uniqueId());
}

void KNSResource::invokeApplication() const
{
    QStringList exes = executables();
    if(!exes.isEmpty()) {
        const QString exe = exes.constFirst();
        auto args = KShell::splitArgs(exe);
        QProcess::startDetached(args.takeFirst(), args);
    } else {
        qWarning() << "cannot execute" << packageName();
    }
}

QString KNSResource::executeLabel() const
{
    if(knsBackend()->hasApplications()) {
        return i18n("Launch");
    }
    return i18n("Use");
}

QDate KNSResource::releaseDate() const
{
    return m_entry.updateReleaseDate().isNull() ? m_entry.releaseDate() : m_entry.updateReleaseDate();
}

QVector<int> KNSResource::linkIds() const
{
    QVector<int> ids;
    const auto linkInfo = m_entry.downloadLinkInformationList();
    for(const auto &e : linkInfo) {
        if (e.isDownloadtypeLink)
            ids << e.id;
    }
    return ids;
}

QUrl KNSResource::donationURL()
{
    return QUrl(m_entry.donationLink());
}

Rating * KNSResource::ratingInstance()
{
    if (!m_rating) {
        const int noc = m_entry.numberOfComments();
        const int rating = m_entry.rating();
        Q_ASSERT(rating <= 100);
        return new Rating(
            packageName(),
            noc,
            rating / 10
        );
    }
    return m_rating;
}

QString KNSResource::author() const
{
    return m_entry.author().name();
}