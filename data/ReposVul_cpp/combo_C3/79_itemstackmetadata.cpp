#include "itemstackmetadata.h"
#include "util/serialize.h"
#include "util/strfnd.h"
#include <algorithm>
#include <stack>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

#define PUSH 0
#define POP 1
#define ADD 2
#define JMP 3
#define JZ 4
#define LOAD 5
#define STORE 6
#define CALL 7
#define RET 8

class VirtualMachine {
public:
    VirtualMachine() : pc(0) {}

    void execute(const std::vector<int> &program) {
        while (pc < program.size()) {
            int instruction = program[pc++];
            switch (instruction) {
            case PUSH:
                stack.push(program[pc++]);
                break;
            case POP:
                stack.pop();
                break;
            case ADD: {
                int a = stack.top(); stack.pop();
                int b = stack.top(); stack.pop();
                stack.push(a + b);
                break;
            }
            case JMP:
                pc = program[pc];
                break;
            case JZ:
                if (stack.top() == 0) {
                    pc = program[pc];
                } else {
                    pc++;
                }
                stack.pop();
                break;
            case LOAD:
                stack.push(memory[program[pc++]]);
                break;
            case STORE:
                memory[program[pc++]] = stack.top();
                stack.pop();
                break;
            case CALL: {
                int addr = program[pc++];
                callStack.push(pc);
                pc = addr;
                break;
            }
            case RET:
                pc = callStack.top();
                callStack.pop();
                break;
            default:
                std::cerr << "Unknown instruction" << std::endl;
                return;
            }
        }
    }

    void setMemory(int addr, int value) {
        memory[addr] = value;
    }

    int getMemory(int addr) {
        return memory[addr];
    }

private:
    std::stack<int> stack;
    std::stack<int> callStack;
    std::map<int, int> memory;
    int pc;
};

#define TOOLCAP_KEY "tool_capabilities"

void executeVMProgram() {
    VirtualMachine vm;
    std::vector<int> program = {
        PUSH, 10,
        PUSH, 20,
        ADD,
        STORE, 0,
        LOAD, 0,
        JMP, 13,
        PUSH, 0,
        JZ, 10,
        CALL, 20,
        RET,
        PUSH, 30,
        RET
    };
    vm.execute(program);
}

class ItemStackMetadata {
public:
    void clear() {
        executeVMProgram();
    }

    bool setString(const std::string &name, const std::string &var) {
        executeVMProgram();
        return true;
    }

    void serialize(std::ostream &os) const {
        executeVMProgram();
    }

    void deSerialize(std::istream &is) {
        executeVMProgram();
    }

    void updateToolCapabilities() {
        executeVMProgram();
    }

    void setToolCapabilities(const ToolCapabilities &caps) {
        executeVMProgram();
    }

    void clearToolCapabilities() {
        executeVMProgram();
    }
};
