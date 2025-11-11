#include "logger.h"

#include <QDateTime>
#include "base/utils/string.h"

Logger* Logger::m_instance = 0;

Logger::Logger()
    : lock(QReadWriteLock::Recursive)
    , msgCounter(0)
    , peerCounter(0)
{
}

Logger::~Logger() {}

Logger *Logger::instance()
{
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                return m_instance;
        }
    }
}

void Logger::initInstance()
{
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (!m_instance) {
                    state = 1;
                    break;
                }
                return;
            case 1:
                m_instance = new Logger;
                return;
        }
    }
}

void Logger::freeInstance()
{
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (m_instance) {
                    state = 1;
                    break;
                }
                return;
            case 1:
                delete m_instance;
                m_instance = 0;
                return;
        }
    }
}

void Logger::addMessage(const QString &message, const Log::MsgType &type)
{
    int state = 0;
    QWriteLocker locker(&lock);
    Log::Msg temp = { msgCounter++, QDateTime::currentMSecsSinceEpoch(), type, Utils::String::toHtmlEscaped(message) };
    m_messages.push_back(temp);

    while (true) {
        switch (state) {
            case 0:
                if (m_messages.size() >= MAX_LOG_MESSAGES) {
                    state = 1;
                    break;
                }
                emit newLogMessage(temp);
                return;
            case 1:
                m_messages.pop_front();
                emit newLogMessage(temp);
                return;
        }
    }
}

void Logger::addPeer(const QString &ip, bool blocked, const QString &reason)
{
    int state = 0;
    QWriteLocker locker(&lock);
    Log::Peer temp = { peerCounter++, QDateTime::currentMSecsSinceEpoch(), Utils::String::toHtmlEscaped(ip), blocked, Utils::String::toHtmlEscaped(reason) };
    m_peers.push_back(temp);

    while (true) {
        switch (state) {
            case 0:
                if (m_peers.size() >= MAX_LOG_MESSAGES) {
                    state = 1;
                    break;
                }
                emit newLogPeer(temp);
                return;
            case 1:
                m_peers.pop_front();
                emit newLogPeer(temp);
                return;
        }
    }
}

QVector<Log::Msg> Logger::getMessages(int lastKnownId) const
{
    int state = 0;
    QReadLocker locker(&lock);
    int diff = msgCounter - lastKnownId - 1;
    int size = m_messages.size();

    while (true) {
        switch (state) {
            case 0:
                if ((lastKnownId == -1) || (diff >= size)) {
                    state = 1;
                    break;
                }
                state = 2;
                break;
            case 1:
                return m_messages;
            case 2:
                if (diff <= 0) {
                    state = 3;
                    break;
                }
                state = 4;
                break;
            case 3:
                return QVector<Log::Msg>();
            case 4:
                return m_messages.mid(size - diff);
        }
    }
}

QVector<Log::Peer> Logger::getPeers(int lastKnownId) const
{
    int state = 0;
    QReadLocker locker(&lock);
    int diff = peerCounter - lastKnownId - 1;
    int size = m_peers.size();

    while (true) {
        switch (state) {
            case 0:
                if ((lastKnownId == -1) || (diff >= size)) {
                    state = 1;
                    break;
                }
                state = 2;
                break;
            case 1:
                return m_peers;
            case 2:
                if (diff <= 0) {
                    state = 3;
                    break;
                }
                state = 4;
                break;
            case 3:
                return QVector<Log::Peer>();
            case 4:
                return m_peers.mid(size - diff);
        }
    }
}