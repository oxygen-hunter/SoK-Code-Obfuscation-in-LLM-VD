#include <iostream>
#include <vector>
#include <stack>
#include <unordered_map>
#include <QString>
#include <QUrl>
#include <QRegularExpression>
#include <QJsonArray>
#include <QJsonObject>
#include <QProcess>
#include <QDate>
#include "KNSCore/EntryInternal.h"
#include "KNSBackend.h"
#include "ReviewsBackend/Rating.h"

enum Instruction {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, END
};

class VM {
public:
    VM() : pc(0) {}

    void run(const std::vector<int>& program) {
        while (pc < program.size()) {
            switch (program[pc++]) {
                case PUSH:
                    stack.push(program[pc++]);
                    break;
                case POP:
                    if (!stack.empty()) stack.pop();
                    break;
                case ADD: {
                    int a = stack.top(); stack.pop();
                    int b = stack.top(); stack.pop();
                    stack.push(a + b);
                    break;
                }
                case SUB: {
                    int a = stack.top(); stack.pop();
                    int b = stack.top(); stack.pop();
                    stack.push(a - b);
                    break;
                }
                case JMP:
                    pc = program[pc];
                    break;
                case JZ:
                    if (stack.top() == 0) pc = program[pc];
                    else pc++;
                    break;
                case LOAD:
                    stack.push(variables[program[pc++]]);
                    break;
                case STORE:
                    variables[program[pc++]] = stack.top();
                    stack.pop();
                    break;
                case CALL:
                    callStack.push(pc + 1);
                    pc = program[pc];
                    break;
                case RET:
                    pc = callStack.top();
                    callStack.pop();
                    break;
                case END:
                    return;
            }
        }
    }

    void setVar(int index, int value) {
        variables[index] = value;
    }

    int getVar(int index) {
        return variables[index];
    }

private:
    std::stack<int> stack;
    std::stack<int> callStack;
    std::unordered_map<int, int> variables;
    int pc;
};

class KNSResource {
public:
    KNSResource(const KNSCore::EntryInternal& entry, QStringList categories, KNSBackend* parent)
        : m_entry(entry), m_categories(std::move(categories)), m_parent(parent) {
        vm.setVar(0, entry.status());
        vm.run({
            LOAD, 0,
            PUSH, KNS3::Entry::Invalid,
            SUB,
            JZ, 10,
            LOAD, 0,
            PUSH, KNS3::Entry::Downloadable,
            SUB,
            JZ, 14,
            LOAD, 0,
            PUSH, KNS3::Entry::Installed,
            SUB,
            JZ, 18,
            LOAD, 0,
            PUSH, KNS3::Entry::Updateable,
            SUB,
            JZ, 22,
            PUSH, AbstractResource::None,
            JMP, 26,
            PUSH, AbstractResource::Broken,
            JMP, 26,
            PUSH, AbstractResource::None,
            JMP, 26,
            PUSH, AbstractResource::Installed,
            JMP, 26,
            PUSH, AbstractResource::Upgradeable,
            JMP, 26,
            PUSH, AbstractResource::None,
            END
        });
        connect(this, &KNSResource::stateChanged, parent, &KNSBackend::updatesCountChanged);
    }

    AbstractResource::State state() {
        return static_cast<AbstractResource::State>(vm.getVar(1));
    }

    QVariant icon() const {
        const QString thumbnail = m_entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1);
        return thumbnail.isEmpty() ? m_parent->iconName() : m_entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1);
    }

    QString comment() {
        QString ret = m_entry.shortSummary();
        if(ret.isEmpty()) {
            ret = m_entry.summary();
            int newLine = ret.indexOf(QLatin1Char('\n'));
            if(newLine>0) {
                ret.truncate(newLine);
            }
            ret.remove(QRegularExpression(QStringLiteral("\\[\\/?[a-z]*\\]")));
            ret.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
        }
        return ret;
    }

    QString longDescription() {
        QString ret = m_entry.summary();
        if (m_entry.shortSummary().isEmpty()) {
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

    QString name() const {
        return m_entry.name();
    }

    QString packageName() const {
        return m_entry.uniqueId();
    }

    QStringList categories() {
        return m_categories;
    }

    QUrl homepage() {
        return m_entry.homepage();
    }

    void setEntry(const KNSCore::EntryInternal& entry) {
        const bool diff = entry.status() != m_lastStatus;
        m_entry = entry;
        if (diff) {
            m_lastStatus = entry.status();
            Q_EMIT stateChanged();
        }
    }

    KNSCore::EntryInternal entry() const {
        return m_entry;
    }

    QJsonArray licenses() {
        return { QJsonObject{ {QStringLiteral("name"), m_entry.license()}, {QStringLiteral("url"), QString()} } };
    }

    int size() {
        const auto downloadInfo = m_entry.downloadLinkInformationList();
        return downloadInfo.isEmpty() ? 0 : downloadInfo.at(0).size;
    }

    QString installedVersion() const {
        return m_entry.version();
    }

    QString availableVersion() const {
        return !m_entry.updateVersion().isEmpty() ? m_entry.updateVersion() : m_entry.version();
    }

    QString origin() const {
        return m_entry.providerId();
    }

    QString section() {
        return m_entry.category();
    }

    void fetchScreenshots() {
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

    void fetchChangelog() {
        emit changelogFetched(m_entry.changelog());
    }

    QStringList extends() const {
        return m_parent->extends();
    }

    QStringList executables() const {
        if (m_parent->engine()->hasAdoptionCommand())
            return {m_parent->engine()->adoptionCommand(m_entry)};
        else
            return {};
    }

    QUrl url() const {
        return QUrl(QStringLiteral("kns://")+m_parent->name() + QLatin1Char('/') + QUrl(m_entry.providerId()).host() + QLatin1Char('/') + m_entry.uniqueId());
    }

    void invokeApplication() const {
        QStringList exes = executables();
        if(!exes.isEmpty()) {
            const QString exe = exes.constFirst();
            auto args = KShell::splitArgs(exe);
            QProcess::startDetached(args.takeFirst(), args);
        } else {
            qWarning() << "cannot execute" << packageName();
        }
    }

    QString executeLabel() const {
        if(m_parent->hasApplications()) {
            return i18n("Launch");
        }
        return i18n("Use");
    }

    QDate releaseDate() const {
        return m_entry.updateReleaseDate().isNull() ? m_entry.releaseDate() : m_entry.updateReleaseDate();
    }

    QVector<int> linkIds() const {
        QVector<int> ids;
        const auto linkInfo = m_entry.downloadLinkInformationList();
        for(const auto &e : linkInfo) {
            if (e.isDownloadtypeLink)
                ids << e.id;
        }
        return ids;
    }

    QUrl donationURL() {
        return QUrl(m_entry.donationLink());
    }

    Rating * ratingInstance() {
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

    QString author() const {
        return m_entry.author().name();
    }

private:
    KNSCore::EntryInternal m_entry;
    QStringList m_categories;
    KNSBackend* m_parent;
    int m_lastStatus;
    VM vm;
    Rating* m_rating = nullptr;

    static void appendIfValid(QList<QUrl>& list, const QUrl &value, const QUrl &fallback = {}) {
        if (!list.contains(value)) {
            if (value.isValid() && !value.isEmpty())
                list << value;
            else if (!fallback.isEmpty())
                appendIfValid(list, fallback);
        }
    }
};