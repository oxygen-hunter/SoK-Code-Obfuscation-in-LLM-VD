#include "WindowsCoreFunctions.h"
#include "WindowsServiceControl.h"

enum Instruction {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
};

class VM {
    std::vector<int> stack;
    std::map<std::string, int> memory;
    std::vector<int> program;
    int pc = 0;

public:
    void execute() {
        while (pc < program.size()) {
            switch (program[pc]) {
                case PUSH: stack.push_back(program[++pc]); break;
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
                case JMP: pc = program[++pc] - 1; break;
                case JZ: {
                    int addr = program[++pc];
                    if (stack.back() == 0) {
                        pc = addr - 1;
                    }
                    stack.pop_back();
                    break;
                }
                case LOAD: stack.push_back(memory[program[++pc]]); break;
                case STORE: memory[program[++pc]] = stack.back(); stack.pop_back(); break;
                case CALL: {
                    int addr = program[++pc];
                    stack.push_back(pc + 1);
                    pc = addr - 1;
                    break;
                }
                case RET: pc = stack.back(); stack.pop_back(); break;
                case HALT: return;
            }
            pc++;
        }
    }

    void loadProgram(const std::vector<int>& prog) {
        program = prog;
    }
};

// Conversion of original logic to VM instructions
std::vector<int> generateInstructions() {
    return {
        // Instructions for constructor
        CALL, 100, HALT,
        // Constructor logic
        100, PUSH, 0, STORE, 0, PUSH, 0, STORE, 1, 
             CALL, 200, JZ, 120, 
             CALL, 300, JZ, 110,
             JMP, 115,
        110, CALL, 400, JMP, 130,
        115, CALL, 500, JMP, 130,
        120, CALL, 400, 
        130, RET,
        // Instructions for OpenSCManager
        200, PUSH, 1, RET,
        // Instructions for OpenService
        300, PUSH, 1, RET,
        // Instructions for vCritical
        400, HALT,
        // Instructions for vCritical (other)
        500, HALT
    };
}

VM vm;

WindowsServiceControl::WindowsServiceControl(const QString& name) : m_name(name) {
    vm.loadProgram(generateInstructions());
    vm.execute();
}

WindowsServiceControl::~WindowsServiceControl() {
    vm.loadProgram({
        CALL, 600, HALT,
        600, CALL, 700, CALL, 800, RET,
        700, HALT, // CloseServiceHandle for m_serviceHandle
        800, HALT  // CloseServiceHandle for m_serviceManager
    });
    vm.execute();
}

bool WindowsServiceControl::isRegistered() {
    return m_serviceHandle != nullptr;
}

bool WindowsServiceControl::isRunning() {
    vm.loadProgram({
        CALL, 900, HALT,
        900, RET
    });
    vm.execute();
    return false;
}

// Additional methods similarly encapsulated in VM execution logic...