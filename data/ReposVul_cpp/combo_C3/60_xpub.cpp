#include "precompiled.hpp"
#include <string.h>
#include <vector>

#define PUSH 0
#define POP 1
#define ADD 2
#define SUB 3
#define JMP 4
#define JZ 5
#define LOAD 6
#define STORE 7
#define CALL 8
#define RET 9
#define HALT 10

class VM {
public:
    VM(const std::vector<int>& program)
        : program(program), pc(0), running(true)
    {
        stack.reserve(256);
    }

    void run() {
        while (running) {
            int instruction = program[pc++];
            execute(instruction);
        }
    }

private:
    std::vector<int> program;
    std::vector<int> stack;
    int pc;
    bool running;

    void execute(int instruction) {
        switch (instruction) {
            case PUSH:
                stack.push_back(program[pc++]);
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
                pc = program[pc];
                break;
            case JZ: {
                int address = program[pc++];
                if (stack.back() == 0) {
                    pc = address;
                }
                stack.pop_back();
                break;
            }
            case LOAD: {
                int index = program[pc++];
                stack.push_back(stack[index]);
                break;
            }
            case STORE: {
                int index = program[pc++];
                stack[index] = stack.back();
                stack.pop_back();
                break;
            }
            case CALL:
                stack.push_back(pc + 1);
                pc = program[pc];
                break;
            case RET:
                pc = stack.back();
                stack.pop_back();
                break;
            case HALT:
                running = false;
                break;
            default:
                throw std::runtime_error("Invalid instruction");
        }
    }
};

std::vector<int> compile() {
    return {
        // Example compiled code
        PUSH, 10,
        PUSH, 20,
        ADD,
        PUSH, 30,
        SUB,
        HALT
    };
}

int main() {
    std::vector<int> program = compile();
    VM vm(program);
    vm.run();
    return 0;
}