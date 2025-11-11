#include "logger.h"

#include <QDateTime>
#include "base/utils/string.h"

Logger* Logger::m_instance = nullptr;

Logger::Logger()
    : lock(QReadWriteLock::Recursive)
{
    msgCounter = generateMsgCounter();
    peerCounter = generatePeerCounter();
}

Logger::~Logger() {}

Logger *Logger::instance()
{
    return m_instance;
}

void Logger::initInstance()
{
    if (!m_instance)
        m_instance = createLogger();
}

void Logger::freeInstance()
{
    if (m_instance) {
        deleteLogger(m_instance);
        m_instance = nullptr;
    }
}

void Logger::addMessage(const QString &message, const Log::MsgType &type)
{
    QWriteLocker locker(&lock);

    Log::Msg temp = { incrementMsgCounter(), QDateTime::currentMSecsSinceEpoch(), type, Utils::String::toHtmlEscaped(message) };
    m_messages.push_back(temp);

    if (m_messages.size() >= MAX_LOG_MESSAGES)
        m_messages.pop_front();

    emit newLogMessage(temp);
}

void Logger::addPeer(const QString &ip, bool blocked, const QString &reason)
{
    QWriteLocker locker(&lock);

    Log::Peer temp = { incrementPeerCounter(), QDateTime::currentMSecsSinceEpoch(), Utils::String::toHtmlEscaped(ip), blocked, Utils::String::toHtmlEscaped(reason) };
    m_peers.push_back(temp);

    if (m_peers.size() >= MAX_LOG_MESSAGES)
        m_peers.pop_front();

    emit newLogPeer(temp);
}

QVector<Log::Msg> Logger::getMessages(int lastKnownId) const
{
    QReadLocker locker(&lock);

    int diff = calculateMsgDiff(lastKnownId);
    int size = getMessageSize();

    if ((lastKnownId == -1) || (diff >= size))
        return m_messages;

    if (diff <= 0)
        return QVector<Log::Msg>();

    return m_messages.mid(size - diff);
}

QVector<Log::Peer> Logger::getPeers(int lastKnownId) const
{
    QReadLocker locker(&lock);

    int diff = calculatePeerDiff(lastKnownId);
    int size = getPeerSize();

    if ((lastKnownId == -1) || (diff >= size))
        return m_peers;

    if (diff <= 0)
        return QVector<Log::Peer>();

    return m_peers.mid(size - diff);
}

int Logger::generateMsgCounter() { return 0; }
int Logger::generatePeerCounter() { return 0; }
int Logger::incrementMsgCounter() { return msgCounter++; }
int Logger::incrementPeerCounter() { return peerCounter++; }
int Logger::calculateMsgDiff(int lastKnownId) const { return msgCounter - lastKnownId - 1; }
int Logger::calculatePeerDiff(int lastKnownId) const { return peerCounter - lastKnownId - 1; }
int Logger::getMessageSize() const { return m_messages.size(); }
int Logger::getPeerSize() const { return m_peers.size(); }
Logger* Logger::createLogger() { return new Logger; }
void Logger::deleteLogger(Logger* logger) { delete logger; }