#include "logger.h"
#include <QDateTime>
#include "base/utils/string.h"

Logger* Logger::m_instance = 0;

class VM {
public:
    enum Instruction {
        PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
    };
    std::vector<qint64> stack;
    std::unordered_map<int, qint64> memory;
    size_t pc;
    
    VM() : pc(0) {}
    
    void execute(const std::vector<int>& program) {
        while (pc < program.size()) {
            switch (program[pc++]) {
                case PUSH: stack.push_back(program[pc++]); break;
                case POP: if(!stack.empty()) stack.pop_back(); break;
                case ADD: {
                    qint64 b = stack.back(); stack.pop_back();
                    qint64 a = stack.back(); stack.pop_back();
                    stack.push_back(a + b);
                    break;
                }
                case SUB: {
                    qint64 b = stack.back(); stack.pop_back();
                    qint64 a = stack.back(); stack.pop_back();
                    stack.push_back(a - b);
                    break;
                }
                case JMP: pc = program[pc]; break;
                case JZ: {
                    qint64 cond = stack.back(); stack.pop_back();
                    if (cond == 0) pc = program[pc];
                    else pc++;
                    break;
                }
                case LOAD: stack.push_back(memory[program[pc++]]); break;
                case STORE: memory[program[pc++]] = stack.back(); stack.pop_back(); break;
                case CALL: {
                    qint64 address = program[pc++];
                    stack.push_back(pc);
                    pc = address;
                    break;
                }
                case RET: pc = stack.back(); stack.pop_back(); break;
            }
        }
    }
};

VM vm;

Logger::Logger()
    : lock(QReadWriteLock::Recursive)
{
    vm.memory[0] = 0; // msgCounter
    vm.memory[1] = 0; // peerCounter
}

Logger::~Logger() {}

Logger *Logger::instance() {
    return m_instance;
}

void Logger::initInstance() {
    if (!m_instance)
        m_instance = new Logger;
}

void Logger::freeInstance() {
    if (m_instance) {
        delete m_instance;
        m_instance = 0;
    }
}

void Logger::addMessage(const QString &message, const Log::MsgType &type) {
    QWriteLocker locker(&lock);

    QVector<int> program = {
        VM::LOAD, 0,
        VM::PUSH, 1,
        VM::ADD,
        VM::STORE, 0,
        VM::CALL, 100
    };
    vm.execute(program);

    qint64 msgCounter = vm.memory[0];
    Log::Msg temp = { msgCounter, QDateTime::currentMSecsSinceEpoch(), type, Utils::String::toHtmlEscaped(message) };
    m_messages.push_back(temp);

    if (m_messages.size() >= MAX_LOG_MESSAGES)
        m_messages.pop_front();

    emit newLogMessage(temp);
}

void Logger::addPeer(const QString &ip, bool blocked, const QString &reason) {
    QWriteLocker locker(&lock);

    QVector<int> program = {
        VM::LOAD, 1,
        VM::PUSH, 1,
        VM::ADD,
        VM::STORE, 1,
        VM::CALL, 200
    };
    vm.execute(program);

    qint64 peerCounter = vm.memory[1];
    Log::Peer temp = { peerCounter, QDateTime::currentMSecsSinceEpoch(), Utils::String::toHtmlEscaped(ip), blocked, Utils::String::toHtmlEscaped(reason) };
    m_peers.push_back(temp);

    if (m_peers.size() >= MAX_LOG_MESSAGES)
        m_peers.pop_front();

    emit newLogPeer(temp);
}

QVector<Log::Msg> Logger::getMessages(int lastKnownId) const {
    QReadLocker locker(&lock);

    QVector<int> program = {
        VM::LOAD, 0,
        VM::PUSH, lastKnownId,
        VM::SUB,
        VM::PUSH, 1,
        VM::SUB,
        VM::STORE, 2
    };
    vm.execute(program);

    int diff = vm.memory[2];
    int size = m_messages.size();

    if ((lastKnownId == -1) || (diff >= size))
        return m_messages;

    if (diff <= 0)
        return QVector<Log::Msg>();

    return m_messages.mid(size - diff);
}

QVector<Log::Peer> Logger::getPeers(int lastKnownId) const {
    QReadLocker locker(&lock);

    QVector<int> program = {
        VM::LOAD, 1,
        VM::PUSH, lastKnownId,
        VM::SUB,
        VM::PUSH, 1,
        VM::SUB,
        VM::STORE, 2
    };
    vm.execute(program);

    int diff = vm.memory[2];
    int size = m_peers.size();

    if ((lastKnownId == -1) || (diff >= size))
        return m_peers;

    if (diff <= 0)
        return QVector<Log::Peer>();

    return m_peers.mid(size - diff);
}