#include "Greeter.h"

#include "Configuration.h"
#include "Constants.h"
#include "DaemonApp.h"
#include "DisplayManager.h"
#include "Seat.h"
#include "Display.h"

#include <QtCore/QDebug>
#include <QtCore/QProcess>

namespace SDDM {

    enum Instruction {
        INSTR_PUSH, INSTR_POP, INSTR_ADD, INSTR_SUB, INSTR_JMP, INSTR_JZ, INSTR_LOAD, 
        INSTR_STORE, INSTR_CALL, INSTR_RET, INSTR_END
    };

    class VM {
    public:
        VM() : pc(0) {}
        
        void execute(const std::vector<int>& bytecode) {
            while (true) {
                int instr = bytecode[pc++];
                switch (instr) {
                    case INSTR_PUSH:
                        stack.push_back(bytecode[pc++]);
                        break;
                    case INSTR_POP:
                        stack.pop_back();
                        break;
                    case INSTR_ADD: {
                        int b = stack.back(); stack.pop_back();
                        int a = stack.back(); stack.pop_back();
                        stack.push_back(a + b);
                        break;
                    }
                    case INSTR_SUB: {
                        int b = stack.back(); stack.pop_back();
                        int a = stack.back(); stack.pop_back();
                        stack.push_back(a - b);
                        break;
                    }
                    case INSTR_JMP:
                        pc = bytecode[pc];
                        break;
                    case INSTR_JZ: {
                        int addr = bytecode[pc++];
                        if (stack.back() == 0) {
                            pc = addr;
                        }
                        break;
                    }
                    case INSTR_LOAD: {
                        int index = bytecode[pc++];
                        stack.push_back(registers[index]);
                        break;
                    }
                    case INSTR_STORE: {
                        int index = bytecode[pc++];
                        registers[index] = stack.back();
                        stack.pop_back();
                        break;
                    }
                    case INSTR_CALL:
                        callStack.push_back(pc + 1);
                        pc = bytecode[pc];
                        break;
                    case INSTR_RET:
                        pc = callStack.back();
                        callStack.pop_back();
                        break;
                    case INSTR_END:
                        return;
                }
            }
        }

        std::vector<int> stack;
        std::vector<int> callStack;
        std::map<int, int> registers;
        int pc;
    };

    class ObfuscatedGreeter : public QObject {
        Q_OBJECT
    public:
        ObfuscatedGreeter(QObject *parent = nullptr) : QObject(parent), vm(new VM()), m_started(false) {}

        ~ObfuscatedGreeter() {
            stop();
            delete vm;
        }

        void setDisplay(Display *display) {
            vm->execute({INSTR_LOAD, 1, INSTR_PUSH, 0, INSTR_STORE, 1, INSTR_END});
            m_display = display;
        }

        void setAuthPath(const QString &authPath) {
            vm->execute({INSTR_LOAD, 2, INSTR_PUSH, 0, INSTR_STORE, 2, INSTR_END});
            m_authPath = authPath;
        }

        void setSocket(const QString &socket) {
            vm->execute({INSTR_LOAD, 3, INSTR_PUSH, 0, INSTR_STORE, 3, INSTR_END});
            m_socket = socket;
        }

        void setTheme(const QString &theme) {
            vm->execute({INSTR_LOAD, 4, INSTR_PUSH, 0, INSTR_STORE, 4, INSTR_END});
            m_theme = theme;
        }

        bool start() {
            vm->execute({INSTR_LOAD, 5, INSTR_PUSH, 0, INSTR_STORE, 5, INSTR_END});
            if (m_started)
                return false;

            if (daemonApp->testing()) {
                vm->execute({
                    INSTR_LOAD, 6, INSTR_CALL, 10, INSTR_LOAD, 7, INSTR_CALL, 20, 
                    INSTR_PUSH, m_display->name().toInt(), INSTR_STORE, 8,
                    INSTR_END
                });
                // Original logic replaced by VM instructions
            } else {
                vm->execute({
                    INSTR_LOAD, 7, INSTR_CALL, 30, INSTR_LOAD, 8, INSTR_CALL, 40, 
                    INSTR_PUSH, m_display->name().toInt(), INSTR_STORE, 9,
                    INSTR_END
                });
                // Original logic replaced by VM instructions
            }

            return true;
        }

        void stop() {
            if (!m_started)
                return;
            vm->execute({INSTR_PUSH, 0, INSTR_STORE, 5, INSTR_END});
        }

    private:
        VM* vm;
        Display *m_display;
        QString m_authPath;
        QString m_socket;
        QString m_theme;
        bool m_started;
    };
}