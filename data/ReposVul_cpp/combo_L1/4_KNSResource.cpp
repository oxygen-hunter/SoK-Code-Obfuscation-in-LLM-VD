/***************************************************************************
 *   Copyright © 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "KNSResource.h"
#include "KNSBackend.h"
#include <KNSCore/Engine>
#include <KShell>
#include <KLocalizedString>
#include <QProcess>
#include <QRegularExpression>

#include "ReviewsBackend/Rating.h"
#include <knewstuff_version.h>

OX7B4DF339::OX7B4DF339(const KNSCore::EntryInternal& OX0F3E3F5A, QStringList OX5C9E5A8F, KNSBackend* OX2F8A7B6E)
    : AbstractResource(OX2F8A7B6E)
    , OXA8D9B1D3(std::move(OX5C9E5A8F))
    , OX0F3E3F5A(OX0F3E3F5A)
    , OX5F4C1D1E(OX0F3E3F5A.status())
{
    connect(this, &OX7B4DF339::stateChanged, OX2F8A7B6E, &KNSBackend::updatesCountChanged);
}

OX7B4DF339::~OX7B4DF339() = default;

AbstractResource::State OX7B4DF339::OX8F4F1E49()
{
    switch(OX0F3E3F5A.status()) {
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

KNSBackend * OX7B4DF339::OX5B2D9F39() const
{
    return qobject_cast<KNSBackend*>(parent());
}

QVariant OX7B4DF339::OX8F9C9DB3() const
{
    const QString OX4DFE9C6B = OX0F3E3F5A.previewUrl(KNSCore::EntryInternal::PreviewSmall1);
    return OX4DFE9C6B.isEmpty() ? OX5B2D9F39()->iconName() : OX0F3E3F5A.previewUrl(KNSCore::EntryInternal::PreviewSmall1);
}

QString OX7B4DF339::OX3EDC3C5D()
{
    QString OX4D4C0D0D = OX0F3E3F5A.shortSummary();
    if(OX4D4C0D0D.isEmpty()) {
        OX4D4C0D0D = OX0F3E3F5A.summary();
        int OX3A5C4E5F = OX4D4C0D0D.indexOf(QLatin1Char('\n'));
        if(OX3A5C4E5F>0) {
            OX4D4C0D0D.truncate(OX3A5C4E5F);
        }
        OX4D4C0D0D.remove(QRegularExpression(QStringLiteral("\\[\\/?[a-z]*\\]")));
        OX4D4C0D0D.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
    }
    return OX4D4C0D0D;
}

QString OX7B4DF339::OX3A5F8E5E()
{
    QString OX4D4C0D0D = OX0F3E3F5A.summary();
    if (OX0F3E3F5A.shortSummary().isEmpty()) {
        const int OX3A5C4E5F = OX4D4C0D0D.indexOf(QLatin1Char('\n'));
        if (OX3A5C4E5F<0)
            OX4D4C0D0D.clear();
        else
            OX4D4C0D0D = OX4D4C0D0D.mid(OX3A5C4E5F+1).trimmed();
    }
    OX4D4C0D0D.remove(QLatin1Char('\r'));
    OX4D4C0D0D.replace(QStringLiteral("[li]"), QStringLiteral("\n* "));
    OX4D4C0D0D.remove(QRegularExpression(QStringLiteral("\\[\\/?[a-z]*\\]")));
    static const QRegularExpression OX0F9E9F3D(QStringLiteral("(^|\\s)(http[-a-zA-Z0-9@:%_\\+.~#?&//=]{2,256}\\.[a-z]{2,4}\\b(\\/[-a-zA-Z0-9@:;%_\\+.~#?&//=]*)?)"), QRegularExpression::CaseInsensitiveOption);
    OX4D4C0D0D.replace(OX0F9E9F3D, QStringLiteral("<a href=\"\\2\">\\2</a>"));
    return OX4D4C0D0D;
}

QString OX7B4DF339::OX3F3B3D2E() const
{
    return OX0F3E3F5A.name();
}

QString OX7B4DF339::OX3A5B4C5F() const
{
    return OX0F3E3F5A.uniqueId();
}

QStringList OX7B4DF339::OX2D4B4C5A()
{
    return OXA8D9B1D3;
}

QUrl OX7B4DF339::OX5F3B3B4B()
{
    return OX0F3E3F5A.homepage();
}

void OX7B4DF339::OX3A4C3D5F(const KNSCore::EntryInternal& OX0F3E3F5A)
{
    const bool OX0F3E9E9F = OX0F3E3F5A.status() != OX5F4C1D1E;
    OX0F3E3F5A = OX0F3E3F5A;
    if (OX0F3E9E9F) {
        OX5F4C1D1E = OX0F3E3F5A.status();
        Q_EMIT stateChanged();
    }
}

KNSCore::EntryInternal OX7B4DF339::OX0F3E3F5A() const
{
    return OX0F3E3F5A;
}

QJsonArray OX7B4DF339::OX8F3E4D3F()
{
    return { QJsonObject{ {QStringLiteral("name"), OX0F3E3F5A.license()}, {QStringLiteral("url"), QString()} } };
}

int OX7B4DF339::OX2F3B4C5A()
{
    const auto OX4F5F3A3B = OX0F3E3F5A.downloadLinkInformationList();
    return OX4F5F3A3B.isEmpty() ? 0 : OX4F5F3A3B.at(0).size;
}

QString OX7B4DF339::OX4D3E5C4F() const
{
    return OX0F3E3F5A.version();
}

QString OX7B4DF339::OX3A5C4E5F() const
{
    return !OX0F3E3F5A.updateVersion().isEmpty() ? OX0F3E3F5A.updateVersion() : OX0F3E3F5A.version();
}

QString OX7B4DF339::OX5F4D3E4B() const
{
    return OX0F3E3F5A.providerId();
}

QString OX7B4DF339::OX4B3C5A3A()
{
    return OX0F3E3F5A.category();
}

static void OX3F5B3C2D(QList<QUrl>& OX0F3E4F5C, const QUrl &OX4C5A3D3B, const QUrl &OX4F5C3E4B = {})
{
    if (!OX0F3E4F5C.contains(OX4C5A3D3B)) {
        if (OX4C5A3D3B.isValid() && !OX4C5A3D3B.isEmpty())
            OX0F3E4F5C << OX4C5A3D3B;
        else if (!OX4F5C3E4B.isEmpty())
            OX3F5B3C2D(OX0F3E4F5C, OX4F5C3E4B);
    }
}

void OX7B4DF339::OX5B4C3F5A()
{
    QList<QUrl> OX8F3E4D3F;
    OX3F5B3C2D(OX8F3E4D3F, QUrl(OX0F3E3F5A.previewUrl(KNSCore::EntryInternal::PreviewSmall1)));
    OX3F5B3C2D(OX8F3E4D3F, QUrl(OX0F3E3F5A.previewUrl(KNSCore::EntryInternal::PreviewSmall2)));
    OX3F5B3C2D(OX8F3E4D3F, QUrl(OX0F3E3F5A.previewUrl(KNSCore::EntryInternal::PreviewSmall3)));

    QList<QUrl> OX2F3B4C5A;
    OX3F5B3C2D(OX2F3B4C5A, QUrl(OX0F3E3F5A.previewUrl(KNSCore::EntryInternal::PreviewBig1)), QUrl(OX0F3E3F5A.previewUrl(KNSCore::EntryInternal::PreviewSmall1)));
    OX3F5B3C2D(OX2F3B4C5A, QUrl(OX0F3E3F5A.previewUrl(KNSCore::EntryInternal::PreviewBig2)), QUrl(OX0F3E3F5A.previewUrl(KNSCore::EntryInternal::PreviewSmall2)));
    OX3F5B3C2D(OX2F3B4C5A, QUrl(OX0F3E3F5A.previewUrl(KNSCore::EntryInternal::PreviewBig3)), QUrl(OX0F3E3F5A.previewUrl(KNSCore::EntryInternal::PreviewSmall3)));

    emit screenshotsFetched(OX8F3E4D3F, OX2F3B4C5A);
}

void OX7B4DF339::OX8F5C4D3F()
{
    emit changelogFetched(OX0F3E3F5A.changelog());
}

QStringList OX7B4DF339::OX3A5B4C5F() const
{
    return OX5B2D9F39()->extends();
}

QStringList OX7B4DF339::OX4F5C3E4B() const
{
    if (OX5B2D9F39()->engine()->hasAdoptionCommand())
        return {OX5B2D9F39()->engine()->adoptionCommand(OX0F3E3F5A)};
    else
        return {};
}

QUrl OX7B4DF339::OX5F3B3B4B() const
{
    return QUrl(QStringLiteral("kns://")+OX5B2D9F39()->name() + QLatin1Char('/') + QUrl(OX0F3E3F5A.providerId()).host() + QLatin1Char('/') + OX0F3E3F5A.uniqueId());
}

void OX7B4DF339::OX8F9C9DB3() const
{
    QStringList OX4C5A3D3B = OX4F5C3E4B();
    if(!OX4C5A3D3B.isEmpty()) {
        const QString OX4DFE9C6B = OX4C5A3D3B.constFirst();
        auto OX3F3B3D2E = KShell::splitArgs(OX4DFE9C6B);
        QProcess::startDetached(OX3F3B3D2E.takeFirst(), OX3F3B3D2E);
    } else {
        qWarning() << "cannot execute" << OX3A5B4C5F();
    }
}

QString OX7B4DF339::OX3A5C4E5F() const
{
    if(OX5B2D9F39()->hasApplications()) {
        return i18n("Launch");
    }
    return i18n("Use");
}

QDate OX7B4DF339::OX3F3B3D2E() const
{
    return OX0F3E3F5A.updateReleaseDate().isNull() ? OX0F3E3F5A.releaseDate() : OX0F3E3F5A.updateReleaseDate();
}

QVector<int> OX7B4DF339::OX3A5B4C5A() const
{
    QVector<int> OX4F5F3A3B;
    const auto OX2D4B4C5A = OX0F3E3F5A.downloadLinkInformationList();
    for(const auto &OX4C5A3D3B : OX2D4B4C5A) {
        if (OX4C5A3D3B.isDownloadtypeLink)
            OX4F5F3A3B << OX4C5A3D3B.id;
    }
    return OX4F5F3A3B;
}

QUrl OX7B4DF339::OX5F3B3B4B()
{
    return QUrl(OX0F3E3F5A.donationLink());
}

Rating * OX7B4DF339::OX7B4DF339()
{
    if (!OX2F8A7B6E) {
        const int OX0F3E9E9F = OX0F3E3F5A.numberOfComments();
        const int OX3F3B3D2E = OX0F3E3F5A.rating();
        Q_ASSERT(OX3F3B3D2E <= 100);
        return new Rating(
            OX3A5B4C5F(),
            OX0F3E9E9F,
            OX3F3B3D2E / 10
        );
    }
    return OX2F8A7B6E;
}

QString OX7B4DF339::OX3F3B3D2E() const
{
    return OX0F3E3F5A.author().name();
}