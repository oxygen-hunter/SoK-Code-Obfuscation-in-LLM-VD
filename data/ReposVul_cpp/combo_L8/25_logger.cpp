#include <QDateTime>
#include "base/utils/string.h"

extern "C" {
    typedef struct {
        int id;
        qint64 timestamp;
        int type;
        char* message;
    } Msg;

    typedef struct {
        int id;
        qint64 timestamp;
        char* ip;
        bool blocked;
        char* reason;
    } Peer;

    void* new_logger_instance();
    void delete_logger_instance(void* instance);
    void add_message(void* instance, const char* message, int type);
    void add_peer(void* instance, const char* ip, bool blocked, const char* reason);
    Msg* get_messages(void* instance, int lastKnownId, int* outSize);
    Peer* get_peers(void* instance, int lastKnownId, int* outSize);
}

class Logger {
public:
    static Logger* instance() {
        if (!m_instance) {
            initInstance();
        }
        return m_instance;
    }

    static void initInstance() {
        if (!m_instance) {
            m_instance = reinterpret_cast<Logger*>(new_logger_instance());
        }
    }

    static void freeInstance() {
        if (m_instance) {
            delete_logger_instance(m_instance);
            m_instance = nullptr;
        }
    }

    void addMessage(const QString &message, const Log::MsgType &type) {
        add_message(m_instance, message.toUtf8().constData(), type);
    }

    void addPeer(const QString &ip, bool blocked, const QString &reason) {
        add_peer(m_instance, ip.toUtf8().constData(), blocked, reason.toUtf8().constData());
    }

    QVector<Log::Msg> getMessages(int lastKnownId) const {
        int size;
        Msg* msgs = get_messages(m_instance, lastKnownId, &size);
        QVector<Log::Msg> vec;
        for (int i = 0; i < size; ++i) {
            vec.append({msgs[i].id, msgs[i].timestamp, msgs[i].type, QString(msgs[i].message)});
        }
        return vec;
    }

    QVector<Log::Peer> getPeers(int lastKnownId) const {
        int size;
        Peer* peers = get_peers(m_instance, lastKnownId, &size);
        QVector<Log::Peer> vec;
        for (int i = 0; i < size; ++i) {
            vec.append({peers[i].id, peers[i].timestamp, QString(peers[i].ip), peers[i].blocked, QString(peers[i].reason)});
        }
        return vec;
    }

private:
    static Logger* m_instance;
};

// Define static member
Logger* Logger::m_instance = nullptr;