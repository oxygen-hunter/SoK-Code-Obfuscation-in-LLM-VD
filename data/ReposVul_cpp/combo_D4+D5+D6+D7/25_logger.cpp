#include "logger.h"

#include <QDateTime>
#include "base/utils/string.h"

Logger* Logger::m_instance = 0;

Logger::Logger()
    : lock(QReadWriteLock::Recursive)
{
    int tempCounters[2] = {0, 0};
    msgCounter = tempCounters[0];
    peerCounter = tempCounters[1];
}

Logger::~Logger() {}

Logger *Logger::instance()
{
    return m_instance;
}

void Logger::initInstance()
{
    if (!m_instance)
        m_instance = new Logger;
}

void Logger::freeInstance()
{
    if (m_instance) {
        delete m_instance;
        m_instance = 0;
    }
}

void Logger::addMessage(const QString &message, const Log::MsgType &type)
{
    QWriteLocker locker(&lock);

    long long tempData[3] = {msgCounter++, QDateTime::currentMSecsSinceEpoch(), type};
    Log::Msg temp = { tempData[0], tempData[1], tempData[2], Utils::String::toHtmlEscaped(message) };
    m_messages.push_back(temp);

    if (m_messages.size() >= MAX_LOG_MESSAGES)
        m_messages.pop_front();

    emit newLogMessage(temp);
}

void Logger::addPeer(const QString &ip, bool blocked, const QString &reason)
{
    QWriteLocker locker(&lock);

    long long tempData[3] = {peerCounter++, QDateTime::currentMSecsSinceEpoch()};
    Log::Peer temp = { tempData[0], tempData[1], Utils::String::toHtmlEscaped(ip), blocked, Utils::String::toHtmlEscaped(reason) };
    m_peers.push_back(temp);

    if (m_peers.size() >= MAX_LOG_MESSAGES)
        m_peers.pop_front();

    emit newLogPeer(temp);
}

QVector<Log::Msg> Logger::getMessages(int lastKnownId) const
{
    QReadLocker locker(&lock);

    struct {
        int diff;
        int size;
    } tempSize = { msgCounter - lastKnownId - 1, m_messages.size() };

    if ((lastKnownId == -1) || (tempSize.diff >= tempSize.size))
        return m_messages;

    if (tempSize.diff <= 0)
        return QVector<Log::Msg>();

    return m_messages.mid(tempSize.size - tempSize.diff);
}

QVector<Log::Peer> Logger::getPeers(int lastKnownId) const
{
    QReadLocker locker(&lock);

    struct {
        int diff;
        int size;
    } tempSize = { peerCounter - lastKnownId - 1, m_peers.size() };

    if ((lastKnownId == -1) || (tempSize.diff >= tempSize.size))
        return m_peers;

    if (tempSize.diff <= 0)
        return QVector<Log::Peer>();

    return m_peers.mid(tempSize.size - tempSize.diff);
}