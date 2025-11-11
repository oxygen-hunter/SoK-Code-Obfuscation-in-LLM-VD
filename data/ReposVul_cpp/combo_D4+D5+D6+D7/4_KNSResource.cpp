#include "KNSResource.h"
#include "KNSBackend.h"
#include <KNSCore/Engine>
#include <KShell>
#include <KLocalizedString>
#include <QProcess>
#include <QRegularExpression>

#include "ReviewsBackend/Rating.h"
#include <knewstuff_version.h>

QStringList globalCategories;
KNSCore::EntryInternal globalEntry;
int globalLastStatus;

KNSResource::KNSResource(const KNSCore::EntryInternal& entry, QStringList categories, KNSBackend* parent)
    : AbstractResource(parent)
{
    globalCategories = std::move(categories);
    globalEntry = entry;
    globalLastStatus = entry.status();
    connect(this, &KNSResource::stateChanged, parent, &KNSBackend::updatesCountChanged);
}

KNSResource::~KNSResource() = default;

AbstractResource::State KNSResource::state()
{
    switch(globalEntry.status()) {
        case KNS3::Entry::Invalid:
            return Broken;
        case KNS3::Entry::Downloadable:
            return None;
        case KNS3::Entry::Installed:
            return Installed;
        case KNS3::Entry::Updateable:
            return Upgradeable;
        case KNS3::Entry::Deleted:
        case KNS3::Entry::Installing:
        case KNS3::Entry::Updating:
            return None;
    }
    return None;
}

KNSBackend * KNSResource::knsBackend() const
{
    return qobject_cast<KNSBackend*>(parent());
}

QVariant KNSResource::icon() const
{
    const QString thumbnail = globalEntry.previewUrl(KNSCore::EntryInternal::PreviewSmall1);
    return thumbnail.isEmpty() ? knsBackend()->iconName() : globalEntry.previewUrl(KNSCore::EntryInternal::PreviewSmall1);
}

QString KNSResource::comment()
{
    QString ret = globalEntry.shortSummary();
    if(ret.isEmpty()) {
        ret = globalEntry.summary();
        int newLine = ret.indexOf(QLatin1Char('\n'));
        if(newLine>0) {
            ret.truncate(newLine);
        }
        ret.remove(QRegularExpression(QStringLiteral("\\[\\/?[a-z]*\\]")));
        ret.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
    }
    return ret;
}

QString KNSResource::longDescription()
{
    QString ret = globalEntry.summary();
    if (globalEntry.shortSummary().isEmpty()) {
        const int newLine = ret.indexOf(QLatin1Char('\n'));
        if (newLine<0)
            ret.clear();
        else
            ret = ret.mid(newLine+1).trimmed();
    }
    ret.remove(QLatin1Char('\r'));
    ret.replace(QStringLiteral("[li]"), QStringLiteral("\n* "));
    // Get rid of all BBCode markup we don't handle above
    ret.remove(QRegularExpression(QStringLiteral("\\[\\/?[a-z]*\\]")));
    // Find anything that looks like a link (but which also is not some html
    // tag value or another already) and make it a link
    static const QRegularExpression urlRegExp(QStringLiteral("(^|\\s)(http[-a-zA-Z0-9@:%_\\+.~#?&//=]{2,256}\\.[a-z]{2,4}\\b(\\/[-a-zA-Z0-9@:;%_\\+.~#?&//=]*)?)"), QRegularExpression::CaseInsensitiveOption);
    ret.replace(urlRegExp, QStringLiteral("<a href=\"\\2\">\\2</a>"));
    return ret;
}

QString KNSResource::name() const
{
    return globalEntry.name();
}

QString KNSResource::packageName() const
{
    return globalEntry.uniqueId();
}

QStringList KNSResource::categories()
{
    return globalCategories;
}

QUrl KNSResource::homepage()
{
    return globalEntry.homepage();
}

void KNSResource::setEntry(const KNSCore::EntryInternal& entry)
{
    const bool diff = entry.status() != globalLastStatus;
    globalEntry = entry;
    if (diff) {
        globalLastStatus = entry.status();
        Q_EMIT stateChanged();
    }
}

KNSCore::EntryInternal KNSResource::entry() const
{
    return globalEntry;
}

QJsonArray KNSResource::licenses()
{
    return { QJsonObject{ {QStringLiteral("name"), globalEntry.license()}, {QStringLiteral("url"), QString()} } };
}

int KNSResource::size()
{
    const auto downloadInfo = globalEntry.downloadLinkInformationList();
    return downloadInfo.isEmpty() ? 0 : downloadInfo.at(0).size;
}

QString KNSResource::installedVersion() const
{
    return globalEntry.version();
}

QString KNSResource::availableVersion() const
{
    return !globalEntry.updateVersion().isEmpty() ? globalEntry.updateVersion() : globalEntry.version();
}

QString KNSResource::origin() const
{
    return globalEntry.providerId();
}

QString KNSResource::section()
{
    return globalEntry.category();
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
    appendIfValid(preview, QUrl(globalEntry.previewUrl(KNSCore::EntryInternal::PreviewSmall1)));
    appendIfValid(preview, QUrl(globalEntry.previewUrl(KNSCore::EntryInternal::PreviewSmall2)));
    appendIfValid(preview, QUrl(globalEntry.previewUrl(KNSCore::EntryInternal::PreviewSmall3)));

    QList<QUrl> screenshots;
    appendIfValid(screenshots, QUrl(globalEntry.previewUrl(KNSCore::EntryInternal::PreviewBig1)), QUrl(globalEntry.previewUrl(KNSCore::EntryInternal::PreviewSmall1)));
    appendIfValid(screenshots, QUrl(globalEntry.previewUrl(KNSCore::EntryInternal::PreviewBig2)), QUrl(globalEntry.previewUrl(KNSCore::EntryInternal::PreviewSmall2)));
    appendIfValid(screenshots, QUrl(globalEntry.previewUrl(KNSCore::EntryInternal::PreviewBig3)), QUrl(globalEntry.previewUrl(KNSCore::EntryInternal::PreviewSmall3)));

    emit screenshotsFetched(preview, screenshots);
}

void KNSResource::fetchChangelog()
{
    emit changelogFetched(globalEntry.changelog());
}

QStringList KNSResource::extends() const
{
    return knsBackend()->extends();
}

QStringList KNSResource::executables() const
{
    if (knsBackend()->engine()->hasAdoptionCommand())
        return {knsBackend()->engine()->adoptionCommand(globalEntry)};
    else
        return {};
}

QUrl KNSResource::url() const
{
    return QUrl(QStringLiteral("kns://")+knsBackend()->name() + QLatin1Char('/') + QUrl(globalEntry.providerId()).host() + QLatin1Char('/') + globalEntry.uniqueId());
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
    return globalEntry.updateReleaseDate().isNull() ? globalEntry.releaseDate() : globalEntry.updateReleaseDate();
}

QVector<int> KNSResource::linkIds() const
{
    QVector<int> ids;
    const auto linkInfo = globalEntry.downloadLinkInformationList();
    for(const auto &e : linkInfo) {
        if (e.isDownloadtypeLink)
            ids << e.id;
    }
    return ids;
}

QUrl KNSResource::donationURL()
{
    return QUrl(globalEntry.donationLink());
}

Rating * KNSResource::ratingInstance()
{
    if (!m_rating) {
        const int noc = globalEntry.numberOfComments();
        const int rating = globalEntry.rating();
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
    return globalEntry.author().name();
}