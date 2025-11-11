#include "corebasichandler.h"
#include "util.h"
#include "logger.h"

class VirtualMachine {
public:
    enum Instruction {
        PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
    };

    VirtualMachine() : pc(0) {}

    void execute(const std::vector<std::pair<Instruction, int>>& program) {
        while (pc < program.size()) {
            auto [instr, operand] = program[pc];
            switch (instr) {
                case PUSH:
                    stack.push_back(operand);
                    break;
                case POP:
                    if (!stack.empty()) stack.pop_back();
                    break;
                case ADD: {
                    if (stack.size() >= 2) {
                        int b = stack.back(); stack.pop_back();
                        int a = stack.back(); stack.pop_back();
                        stack.push_back(a + b);
                    }
                    break;
                }
                case SUB: {
                    if (stack.size() >= 2) {
                        int b = stack.back(); stack.pop_back();
                        int a = stack.back(); stack.pop_back();
                        stack.push_back(a - b);
                    }
                    break;
                }
                case JMP:
                    pc = operand - 1;
                    break;
                case JZ:
                    if (!stack.empty() && stack.back() == 0) {
                        pc = operand - 1;
                    }
                    break;
                case LOAD:
                    stack.push_back(registers[operand]);
                    break;
                case STORE:
                    if (!stack.empty()) {
                        registers[operand] = stack.back();
                        stack.pop_back();
                    }
                    break;
                case CALL:
                    call_stack.push_back(pc);
                    pc = operand - 1;
                    break;
                case RET:
                    if (!call_stack.empty()) {
                        pc = call_stack.back();
                        call_stack.pop_back();
                    }
                    break;
            }
            ++pc;
        }
    }

private:
    std::vector<int> stack;
    std::vector<int> registers{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<int> call_stack;
    size_t pc;
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
    std::vector<std::pair<VirtualMachine::Instruction, int>> program = {
        {VirtualMachine::LOAD, 0}, 
        {VirtualMachine::CALL, 1}, 
        {VirtualMachine::RET, 0}
    };
    vm.execute(program);
    return network()->serverDecode(string);
}

QStringList CoreBasicHandler::serverDecode(const QList<QByteArray> &stringlist) {
    VirtualMachine vm;
    std::vector<std::pair<VirtualMachine::Instruction, int>> program = {
        {VirtualMachine::LOAD, 0}, 
        {VirtualMachine::CALL, 1}, 
        {VirtualMachine::RET, 0}
    };
    vm.execute(program);
    QStringList list;
    foreach(QByteArray s, stringlist) list << network()->serverDecode(s);
    return list;
}

QString CoreBasicHandler::channelDecode(const QString &bufferName, const QByteArray &string) {
    VirtualMachine vm;
    std::vector<std::pair<VirtualMachine::Instruction, int>> program = {
        {VirtualMachine::LOAD, 0}, 
        {VirtualMachine::CALL, 1}, 
        {VirtualMachine::RET, 0}
    };
    vm.execute(program);
    return network()->channelDecode(bufferName, string);
}

QStringList CoreBasicHandler::channelDecode(const QString &bufferName, const QList<QByteArray> &stringlist) {
    VirtualMachine vm;
    std::vector<std::pair<VirtualMachine::Instruction, int>> program = {
        {VirtualMachine::LOAD, 0}, 
        {VirtualMachine::CALL, 1}, 
        {VirtualMachine::RET, 0}
    };
    vm.execute(program);
    QStringList list;
    foreach(QByteArray s, stringlist) list << network()->channelDecode(bufferName, s);
    return list;
}

QString CoreBasicHandler::userDecode(const QString &userNick, const QByteArray &string) {
    VirtualMachine vm;
    std::vector<std::pair<VirtualMachine::Instruction, int>> program = {
        {VirtualMachine::LOAD, 0}, 
        {VirtualMachine::CALL, 1}, 
        {VirtualMachine::RET, 0}
    };
    vm.execute(program);
    return network()->userDecode(userNick, string);
}

QStringList CoreBasicHandler::userDecode(const QString &userNick, const QList<QByteArray> &stringlist) {
    VirtualMachine vm;
    std::vector<std::pair<VirtualMachine::Instruction, int>> program = {
        {VirtualMachine::LOAD, 0}, 
        {VirtualMachine::CALL, 1}, 
        {VirtualMachine::RET, 0}
    };
    vm.execute(program);
    QStringList list;
    foreach(QByteArray s, stringlist) list << network()->userDecode(userNick, s);
    return list;
}

/*** ***/

QByteArray CoreBasicHandler::serverEncode(const QString &string) {
    VirtualMachine vm;
    std::vector<std::pair<VirtualMachine::Instruction, int>> program = {
        {VirtualMachine::LOAD, 0}, 
        {VirtualMachine::CALL, 2}, 
        {VirtualMachine::RET, 0}
    };
    vm.execute(program);
    return network()->serverEncode(string);
}

QList<QByteArray> CoreBasicHandler::serverEncode(const QStringList &stringlist) {
    VirtualMachine vm;
    std::vector<std::pair<VirtualMachine::Instruction, int>> program = {
        {VirtualMachine::LOAD, 0}, 
        {VirtualMachine::CALL, 2}, 
        {VirtualMachine::RET, 0}
    };
    vm.execute(program);
    QList<QByteArray> list;
    foreach(QString s, stringlist) list << network()->serverEncode(s);
    return list;
}

QByteArray CoreBasicHandler::channelEncode(const QString &bufferName, const QString &string) {
    VirtualMachine vm;
    std::vector<std::pair<VirtualMachine::Instruction, int>> program = {
        {VirtualMachine::LOAD, 0}, 
        {VirtualMachine::CALL, 2}, 
        {VirtualMachine::RET, 0}
    };
    vm.execute(program);
    return network()->channelEncode(bufferName, string);
}

QList<QByteArray> CoreBasicHandler::channelEncode(const QString &bufferName, const QStringList &stringlist) {
    VirtualMachine vm;
    std::vector<std::pair<VirtualMachine::Instruction, int>> program = {
        {VirtualMachine::LOAD, 0}, 
        {VirtualMachine::CALL, 2}, 
        {VirtualMachine::RET, 0}
    };
    vm.execute(program);
    QList<QByteArray> list;
    foreach(QString s, stringlist) list << network()->channelEncode(bufferName, s);
    return list;
}

QByteArray CoreBasicHandler::userEncode(const QString &userNick, const QString &string) {
    VirtualMachine vm;
    std::vector<std::pair<VirtualMachine::Instruction, int>> program = {
        {VirtualMachine::LOAD, 0}, 
        {VirtualMachine::CALL, 2}, 
        {VirtualMachine::RET, 0}
    };
    vm.execute(program);
    return network()->userEncode(userNick, string);
}

QList<QByteArray> CoreBasicHandler::userEncode(const QString &userNick, const QStringList &stringlist) {
    VirtualMachine vm;
    std::vector<std::pair<VirtualMachine::Instruction, int>> program = {
        {VirtualMachine::LOAD, 0}, 
        {VirtualMachine::CALL, 2}, 
        {VirtualMachine::RET, 0}
    };
    vm.execute(program);
    QList<QByteArray> list;
    foreach(QString s, stringlist) list << network()->userEncode(userNick, s);
    return list;
}

BufferInfo::Type CoreBasicHandler::typeByTarget(const QString &target) const {
    VirtualMachine vm;
    std::vector<std::pair<VirtualMachine::Instruction, int>> program = {
        {VirtualMachine::JZ, 2}, 
        {VirtualMachine::PUSH, BufferInfo::StatusBuffer}, 
        {VirtualMachine::JMP, 5}, 
        {VirtualMachine::LOAD, 0}, 
        {VirtualMachine::CALL, 3}, 
        {VirtualMachine::RET, 0}
    };
    vm.execute(program);
    if (target.isEmpty())
        return BufferInfo::StatusBuffer;
    if (network()->isChannelName(target))
        return BufferInfo::ChannelBuffer;
    return BufferInfo::QueryBuffer;
}

void CoreBasicHandler::putCmd(const QString &cmd, const QByteArray &param, const QByteArray &prefix) {
    VirtualMachine vm;
    std::vector<std::pair<VirtualMachine::Instruction, int>> program = {
        {VirtualMachine::LOAD, 0}, 
        {VirtualMachine::CALL, 4}, 
        {VirtualMachine::RET, 0}
    };
    vm.execute(program);
    QList<QByteArray> list;
    list << param;
    emit putCmd(cmd, list, prefix);
}