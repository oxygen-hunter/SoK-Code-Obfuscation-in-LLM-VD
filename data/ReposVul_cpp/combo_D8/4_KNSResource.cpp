#include "KNSResource.h"
#include "KNSBackend.h"
#include <KNSCore/Engine>
#include <KShell>
#include <KLocalizedString>
#include <QProcess>
#include <QRegularExpression>

#include "ReviewsBackend/Rating.h"
#include <knewstuff_version.h>

static const KNSCore::EntryInternal& getEntry(const KNSCore::EntryInternal& entry) { return entry; }
static QStringList getCategories(QStringList categories) { return std::move(categories); }
static KNSBackend* getParent(KNSBackend* parent) { return parent; }

KNSResource::KNSResource(const KNSCore::EntryInternal& entry, QStringList categories, KNSBackend* parent)
    : AbstractResource(getParent(parent))
    , m_categories(getCategories(categories))
    , m_entry(getEntry(entry))
    , m_lastStatus(getEntry(entry).status())
{
    connect(this, &KNSResource::stateChanged, getParent(parent), &KNSBackend::updatesCountChanged);
}

KNSResource::~KNSResource() = default;

AbstractResource::State KNSResource::state()
{
    switch(getEntry(m_entry).status()) {
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
    return qobject_cast<KNSBackend*>(getParent(parent()));
}

QVariant KNSResource::icon() const
{
    const QString thumbnail = getEntry(m_entry).previewUrl(KNSCore::EntryInternal::PreviewSmall1);
    return thumbnail.isEmpty() ? knsBackend()->iconName() : getEntry(m_entry).previewUrl(KNSCore::EntryInternal::PreviewSmall1);
}

QString KNSResource::comment()
{
    QString ret = getEntry(m_entry).shortSummary();
    if(ret.isEmpty()) {
        ret = getEntry(m_entry).summary();
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
    QString ret = getEntry(m_entry).summary();
    if (getEntry(m_entry).shortSummary().isEmpty()) {
        const int newLine = ret.indexOf(QLatin1Char('\n'));
        if (newLine<0)
            ret.clear();
        else
            ret = ret.mid(newLine+1).trimmed();
    }
    ret.remove(QLatin1Char('\r'));
    ret.replace(QStringLiteral("[li]"), QStringLiteral("\n* "));
    ret.remove(QRegularExpression(QStringLiteral("\\[\\/?[a-z]*\\]")));
    static const QRegularExpression urlRegExp(QStringLiteral("(^|\\s)(http[-a-zA-Z0-9@:%_\\+.~#?&//=]{2,256}\\.[a-z]{2,4}\\b(\\/[-a-zA-Z0-9@:;%_\\+.~#?&//=]*)?)"), QRegularExpression::CaseInsensitiveOption);
    ret.replace(urlRegExp, QStringLiteral("<a href=\"\\2\">\\2</a>"));
    return ret;
}

QString KNSResource::name() const
{
    return getEntry(m_entry).name();
}

QString KNSResource::packageName() const
{
    return getEntry(m_entry).uniqueId();
}

QStringList KNSResource::categories()
{
    return m_categories;
}

QUrl KNSResource::homepage()
{
    return getEntry(m_entry).homepage();
}

void KNSResource::setEntry(const KNSCore::EntryInternal& entry)
{
    const bool diff = getEntry(entry).status() != m_lastStatus;
    m_entry = getEntry(entry);
    if (diff) {
        m_lastStatus = getEntry(entry).status();
        Q_EMIT stateChanged();
    }
}

KNSCore::EntryInternal KNSResource::entry() const
{
    return getEntry(m_entry);
}

QJsonArray KNSResource::licenses()
{
    return { QJsonObject{ {QStringLiteral("name"), getEntry(m_entry).license()}, {QStringLiteral("url"), QString()} } };
}

int KNSResource::size()
{
    const auto downloadInfo = getEntry(m_entry).downloadLinkInformationList();
    return downloadInfo.isEmpty() ? 0 : downloadInfo.at(0).size;
}

QString KNSResource::installedVersion() const
{
    return getEntry(m_entry).version();
}

QString KNSResource::availableVersion() const
{
    return !getEntry(m_entry).updateVersion().isEmpty() ? getEntry(m_entry).updateVersion() : getEntry(m_entry).version();
}

QString KNSResource::origin() const
{
    return getEntry(m_entry).providerId();
}

QString KNSResource::section()
{
    return getEntry(m_entry).category();
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
    appendIfValid(preview, QUrl(getEntry(m_entry).previewUrl(KNSCore::EntryInternal::PreviewSmall1)));
    appendIfValid(preview, QUrl(getEntry(m_entry).previewUrl(KNSCore::EntryInternal::PreviewSmall2)));
    appendIfValid(preview, QUrl(getEntry(m_entry).previewUrl(KNSCore::EntryInternal::PreviewSmall3)));

    QList<QUrl> screenshots;
    appendIfValid(screenshots, QUrl(getEntry(m_entry).previewUrl(KNSCore::EntryInternal::PreviewBig1)), QUrl(getEntry(m_entry).previewUrl(KNSCore::EntryInternal::PreviewSmall1)));
    appendIfValid(screenshots, QUrl(getEntry(m_entry).previewUrl(KNSCore::EntryInternal::PreviewBig2)), QUrl(getEntry(m_entry).previewUrl(KNSCore::EntryInternal::PreviewSmall2)));
    appendIfValid(screenshots, QUrl(getEntry(m_entry).previewUrl(KNSCore::EntryInternal::PreviewBig3)), QUrl(getEntry(m_entry).previewUrl(KNSCore::EntryInternal::PreviewSmall3)));

    emit screenshotsFetched(preview, screenshots);
}

void KNSResource::fetchChangelog()
{
    emit changelogFetched(getEntry(m_entry).changelog());
}

QStringList KNSResource::extends() const
{
    return knsBackend()->extends();
}

QStringList KNSResource::executables() const
{
    if (knsBackend()->engine()->hasAdoptionCommand())
        return {knsBackend()->engine()->adoptionCommand(getEntry(m_entry))};
    else
        return {};
}

QUrl KNSResource::url() const
{
    return QUrl(QStringLiteral("kns://")+knsBackend()->name() + QLatin1Char('/') + QUrl(getEntry(m_entry).providerId()).host() + QLatin1Char('/') + getEntry(m_entry).uniqueId());
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
    return getEntry(m_entry).updateReleaseDate().isNull() ? getEntry(m_entry).releaseDate() : getEntry(m_entry).updateReleaseDate();
}

QVector<int> KNSResource::linkIds() const
{
    QVector<int> ids;
    const auto linkInfo = getEntry(m_entry).downloadLinkInformationList();
    for(const auto &e : linkInfo) {
        if (e.isDownloadtypeLink)
            ids << e.id;
    }
    return ids;
}

QUrl KNSResource::donationURL()
{
    return QUrl(getEntry(m_entry).donationLink());
}

Rating * KNSResource::ratingInstance()
{
    if (!m_rating) {
        const int noc = getEntry(m_entry).numberOfComments();
        const int rating = getEntry(m_entry).rating();
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
    return getEntry(m_entry).author().name();
}