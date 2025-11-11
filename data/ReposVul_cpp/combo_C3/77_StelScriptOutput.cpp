#include <QDir>
#include <QDebug>
#include <QSettings>
#include "StelScriptOutput.hpp"
#include "StelApp.hpp"

enum Instruction {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
};

class VM {
    QVector<int> stack;
    int pc;
    QVector<int> program;

public:
    VM(const QVector<int>& bytecode) : pc(0), program(bytecode) {}

    void run() {
        while (pc < program.size()) {
            switch (program[pc++]) {
                case PUSH: stack.push_back(program[pc++]); break;
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
                case JMP: pc = program[pc]; break;
                case JZ: if (stack.back() == 0) pc = program[pc]; else pc++; break;
                case LOAD: stack.push_back(program[program[pc++]]); break;
                case STORE: program[program[pc++]] = stack.back(); stack.pop_back(); break;
                case CALL: {
                    int addr = program[pc++];
                    stack.push_back(pc);
                    pc = addr;
                    break;
                }
                case RET: pc = stack.back(); stack.pop_back(); break;
            }
        }
    }
};

QFile StelScriptOutput::outputFile;
QString StelScriptOutput::outputText;

void StelScriptOutput::init(const QString& outputFilePath) {
    QVector<int> bytecode = {
        // PUSH outputFilePath
        PUSH, reinterpret_cast<int>(&outputFilePath),
        // STORE outputFile
        STORE, 0,
        // CALL openFile
        CALL, 10,
        // RET
        RET
    };
    VM(bytecode).run();
}

void StelScriptOutput::deinit() {
    QVector<int> bytecode = {
        // CALL closeFile
        CALL, 20,
        // RET
        RET
    };
    VM(bytecode).run();
}

void StelScriptOutput::writeLog(QString msg) {
    QVector<int> bytecode = {
        // PUSH msg
        PUSH, reinterpret_cast<int>(&msg),
        // CALL appendNewLine
        CALL, 30,
        // CALL writeToFile
        CALL, 40,
        // CALL appendOutputText
        CALL, 50,
        // RET
        RET
    };
    VM(bytecode).run();
}

void StelScriptOutput::reset(void) {
    QVector<int> bytecode = {
        // CALL resizeFile
        CALL, 60,
        // CALL resetOutputText
        CALL, 70,
        // RET
        RET
    };
    VM(bytecode).run();
}

void StelScriptOutput::saveOutputAs(const QString &name) {
    QVector<int> bytecode = {
        // PUSH name
        PUSH, reinterpret_cast<int>(&name),
        // CALL processFileName
        CALL, 80,
        // CALL saveToFile
        CALL, 90,
        // RET
        RET
    };
    VM(bytecode).run();
}