#include <QDir>
#include <QDebug>
#include <QSettings>
#include "StelScriptOutput.hpp"
#include "StelApp.hpp"

enum Instruction {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
};

class VM {
public:
    VM() : pc(0) {}

    void execute(const std::vector<int>& bytecode) {
        while (pc < bytecode.size()) {
            switch (bytecode[pc]) {
                case PUSH: stack.push_back(bytecode[++pc]); break;
                case POP: stack.pop_back(); break;
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
                case JMP: pc = bytecode[++pc] - 1; break;
                case JZ: {
                    int addr = bytecode[++pc];
                    if (stack.back() == 0) pc = addr - 1;
                    break;
                }
                case LOAD: stack.push_back(bytecode[++pc]); break;
                case STORE: {
                    int val = stack.back(); stack.pop_back();
                    memory[bytecode[++pc]] = val;
                    break;
                }
                case CALL: {
                    callStack.push_back(pc + 1);
                    pc = bytecode[++pc] - 1;
                    break;
                }
                case RET: {
                    pc = callStack.back();
                    callStack.pop_back();
                    break;
                }
                default: break;
            }
            pc++;
        }
    }

    std::vector<int>& getStack() { return stack; }
    int& atMemory(int index) { return memory[index]; }

private:
    int pc;
    std::vector<int> stack;
    std::vector<int> memory{std::vector<int>(256)};
    std::vector<int> callStack;
};

QFile StelScriptOutput::outputFile;
QString StelScriptOutput::outputText;

void StelScriptOutput::init(const QString& outputFilePath)
{
    VM vm;
    std::vector<int> bytecode = {
        PUSH, reinterpret_cast<int>(&outputFile),
        CALL, 100, // Call openFile
        RET,

        100, // openFile:
        LOAD, reinterpret_cast<int>(&outputFilePath),
        CALL, 200, // Check if open
        JZ, 300, // Jump to error if not open
        RET,

        200, // open:
        PUSH, QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered,
        CALL, 400,
        RET,

        300, // error:
        LOAD, reinterpret_cast<int>("ERROR: Cannot open file"),
        CALL, 500,
        RET,

        400, // fileOpen:
        LOAD, reinterpret_cast<int>(&qDebug),
        CALL, 600,
        RET,

        500, // logError:
        POP, // Log error
        RET,

        600, // debug:
        POP,
        RET,
    };
    vm.execute(bytecode);
}

void StelScriptOutput::deinit()
{
    VM vm;
    std::vector<int> bytecode = {
        PUSH, reinterpret_cast<int>(&outputFile),
        CALL, 100, // Call closeFile
        RET,

        100, // closeFile:
        CALL, 200,
        RET,

        200, // close:
        LOAD, reinterpret_cast<int>(&outputFile),
        CALL, 300, // Close
        RET,

        300, // fileClose:
        RET,
    };
    vm.execute(bytecode);
}

void StelScriptOutput::writeLog(QString msg)
{
    VM vm;
    std::vector<int> bytecode = {
        PUSH, reinterpret_cast<int>(&msg),
        CALL, 100, // Append newline
        RET,

        100, // appendNewline:
        CALL, 200, // Write
        RET,

        200, // write:
        LOAD, reinterpret_cast<int>(&outputFile),
        CALL, 300,
        RET,

        300, // fileWrite:
        RET,
    };
    vm.execute(bytecode);
}

void StelScriptOutput::reset(void)
{
    VM vm;
    std::vector<int> bytecode = {
        PUSH, reinterpret_cast<int>(&outputFile),
        CALL, 100, // Resize
        RET,

        100, // resize:
        PUSH, 0,
        CALL, 200,
        RET,

        200, // fileResize:
        RET,
    };
    vm.execute(bytecode);
}

void StelScriptOutput::saveOutputAs(const QString &name)
{
    VM vm;
    std::vector<int> bytecode = {
        PUSH, reinterpret_cast<int>(&name),
        CALL, 100, // Save as
        RET,

        100, // saveAs:
        CALL, 200, // Check config.ini
        JZ, 300, // If contains "config.ini", jump to warning
        CALL, 400, // Open new file
        RET,

        200, // checkConfig:
        POP,
        RET,

        300, // warning:
        CALL, 500,
        RET,

        400, // openNewFile:
        LOAD, reinterpret_cast<int>(&qDebug),
        CALL, 600,
        RET,

        500, // logWarning:
        POP,
        RET,

        600, // debug:
        POP,
        RET,
    };
    vm.execute(bytecode);
}