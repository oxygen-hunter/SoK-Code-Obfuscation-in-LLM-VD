#include "socketlinereader.h"

class VirtualMachine {
public:
    enum Instruction {
        PUSH,
        POP,
        ADD,
        SUB,
        JMP,
        JZ,
        LOAD,
        STORE,
        CALL,
        RET,
        HALT
    };

    VirtualMachine() : pc(0) {}

    void execute(const std::vector<int>& bytecode) {
        while (pc < bytecode.size()) {
            switch (bytecode[pc]) {
                case PUSH:
                    stack.push_back(bytecode[++pc]);
                    break;
                case POP:
                    stack.pop_back();
                    break;
                case ADD: {
                    int b = stack.back(); stack.pop_back();
                    int a = stack.back(); stack.pop_back();
                    stack.push_back(a + b);
                    break;
                }
                case SUB: {
                    int b = stack.back(); stack.pop_back();
                    int a = stack.back(); stack.pop_back();
                    stack.push_back(a - b);
                    break;
                }
                case JMP:
                    pc = bytecode[++pc];
                    continue;
                case JZ:
                    if (stack.back() == 0) {
                        pc = bytecode[++pc];
                        continue;
                    } else {
                        ++pc;
                    }
                    break;
                case LOAD:
                    stack.push_back(registers[bytecode[++pc]]);
                    break;
                case STORE:
                    registers[bytecode[++pc]] = stack.back();
                    break;
                case CALL:
                    callStack.push_back(pc + 1);
                    pc = bytecode[++pc];
                    continue;
                case RET:
                    pc = callStack.back();
                    callStack.pop_back();
                    continue;
                case HALT:
                    return;
            }
            ++pc;
        }
    }

private:
    std::vector<int> stack;
    std::vector<int> registers = std::vector<int>(256, 0);
    std::vector<int> callStack;
    int pc;
};

SocketLineReader::SocketLineReader(QSslSocket* socket, QObject* parent)
    : QObject(parent)
    , m_socket(socket)
{
    connect(m_socket, &QIODevice::readyRead,
            this, &SocketLineReader::dataReceived);
}

void SocketLineReader::dataReceived()
{
    VirtualMachine vm;
    std::vector<int> bytecode = {
        VirtualMachine::PUSH, (int)m_socket,
        VirtualMachine::CALL, 10,
        VirtualMachine::HALT,
        VirtualMachine::PUSH, 1, // Address to jump to if line is long enough
        VirtualMachine::LOAD, 0, // Read line and check length
        VirtualMachine::JZ, 15,  // Jump if line is not long enough
        VirtualMachine::CALL, 20, // Enqueue the line
        VirtualMachine::JMP, 10,  // Loop
        VirtualMachine::RET,      // Return from readyRead check
        VirtualMachine::HALT,
    };
    vm.execute(bytecode);
}