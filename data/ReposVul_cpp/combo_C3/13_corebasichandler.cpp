#include "corebasichandler.h"
#include "util.h"
#include "logger.h"

// Define opcodes for the VM
enum Opcode {
    NOP, PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
};

// Simple Stack-based Virtual Machine
class VirtualMachine {
public:
    VirtualMachine() : pc(0) {}

    void execute(const std::vector<int>& program) {
        while (pc < program.size()) {
            switch (program[pc++]) {
                case NOP: break;
                case PUSH: stack.push_back(program[pc++]); break;
                case POP: if (!stack.empty()) stack.pop_back(); break;
                case ADD: binary_op(std::plus<int>()); break;
                case SUB: binary_op(std::minus<int>()); break;
                case JMP: pc = program[pc]; break;
                case JZ: if (stack.back() == 0) pc = program[pc]; else pc++; break;
                case LOAD: stack.push_back(registers[program[pc++]]); break;
                case STORE: registers[program[pc++]] = stack.back(); stack.pop_back(); break;
                case CALL: call(program[pc++]); break;
                case RET: return;
                default: break;
            }
        }
    }

private:
    std::vector<int> stack;
    int registers[10] = {0};
    size_t pc;

    template<typename Op>
    void binary_op(Op op) {
        int b = stack.back(); stack.pop_back();
        int a = stack.back(); stack.pop_back();
        stack.push_back(op(a, b));
    }

    void call(int func) {
        int ret_addr = pc;
        execute(functions[func]);
        pc = ret_addr;
    }

    std::vector<std::vector<int>> functions = {
        // serverDecode functions
        {PUSH, 0, CALL, 1, RET},
        // Sample function for decode logic
        {LOAD, 0, STORE, 1, RET},
    };
};

CoreBasicHandler::CoreBasicHandler(CoreNetwork *parent)
    : BasicHandler(parent),
    _network(parent)
{
    connect(this, SIGNAL(displayMsg(Message::Type, BufferInfo::Type, const QString &, const QString &, const QString &, Message::Flags)),
        network(), SLOT(displayMsg(Message::Type, BufferInfo::Type, const QString &, const QString &, const QString &, Message::Flags)));

    connect(this, SIGNAL(putCmd(QString, const QList<QByteArray> &, const QByteArray &)),
        network(), SLOT(putCmd(QString, const QList<QByteArray> &, const QByteArray &)));

    connect(this, SIGNAL(putCmd(QString, const QList<QList<QByteArray>> &, const QByteArray &)),
        network(), SLOT(putCmd(QString, const QList<QList<QByteArray>> &, const QByteArray &)));

    connect(this, SIGNAL(putRawLine(const QByteArray &)),
        network(), SLOT(putRawLine(const QByteArray &)));
}

QString CoreBasicHandler::serverDecode(const QByteArray &string) {
    VirtualMachine vm;
    std::vector<int> bytecode = {PUSH, 1, CALL, 0, RET};
    vm.execute(bytecode);
    return network()->serverDecode(string);
}

QStringList CoreBasicHandler::serverDecode(const QList<QByteArray> &stringlist) {
    QStringList list;
    foreach(QByteArray s, stringlist) list << network()->serverDecode(s);
    return list;
}

QString CoreBasicHandler::channelDecode(const QString &bufferName, const QByteArray &string) {
    return network()->channelDecode(bufferName, string);
}

QStringList CoreBasicHandler::channelDecode(const QString &bufferName, const QList<QByteArray> &stringlist) {
    QStringList list;
    foreach(QByteArray s, stringlist) list << network()->channelDecode(bufferName, s);
    return list;
}

QString CoreBasicHandler::userDecode(const QString &userNick, const QByteArray &string) {
    return network()->userDecode(userNick, string);
}

QStringList CoreBasicHandler::userDecode(const QString &userNick, const QList<QByteArray> &stringlist) {
    QStringList list;
    foreach(QByteArray s, stringlist) list << network()->userDecode(userNick, s);
    return list;
}

QByteArray CoreBasicHandler::serverEncode(const QString &string) {
    return network()->serverEncode(string);
}

QList<QByteArray> CoreBasicHandler::serverEncode(const QStringList &stringlist) {
    QList<QByteArray> list;
    foreach(QString s, stringlist) list << network()->serverEncode(s);
    return list;
}

QByteArray CoreBasicHandler::channelEncode(const QString &bufferName, const QString &string) {
    return network()->channelEncode(bufferName, string);
}

QList<QByteArray> CoreBasicHandler::channelEncode(const QString &bufferName, const QStringList &stringlist) {
    QList<QByteArray> list;
    foreach(QString s, stringlist) list << network()->channelEncode(bufferName, s);
    return list;
}

QByteArray CoreBasicHandler::userEncode(const QString &userNick, const QString &string) {
    return network()->userEncode(userNick, string);
}

QList<QByteArray> CoreBasicHandler::userEncode(const QString &userNick, const QStringList &stringlist) {
    QList<QByteArray> list;
    foreach(QString s, stringlist) list << network()->userEncode(userNick, s);
    return list;
}

BufferInfo::Type CoreBasicHandler::typeByTarget(const QString &target) const {
    if (target.isEmpty())
        return BufferInfo::StatusBuffer;

    if (network()->isChannelName(target))
        return BufferInfo::ChannelBuffer;

    return BufferInfo::QueryBuffer;
}

void CoreBasicHandler::putCmd(const QString &cmd, const QByteArray &param, const QByteArray &prefix) {
    QList<QByteArray> list;
    list << param;
    emit putCmd(cmd, list, prefix);
}