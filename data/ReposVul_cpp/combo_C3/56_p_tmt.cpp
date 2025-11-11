#include <iostream>
#include <vector>
#include <stack>
#include <cstring>

#define PUSH 0
#define POP 1
#define ADD 2
#define SUB 3
#define JMP 4
#define JZ 5
#define LOAD 6
#define STORE 7
#define HALT 8

class VirtualMachine {
public:
    void execute(const std::vector<int>& bytecode) {
        std::stack<int> stack;
        int pc = 0;
        while (pc < bytecode.size()) {
            switch (bytecode[pc]) {
                case PUSH:
                    stack.push(bytecode[++pc]);
                    break;
                case POP:
                    if (!stack.empty()) stack.pop();
                    break;
                case ADD: {
                    int b = stack.top(); stack.pop();
                    int a = stack.top(); stack.pop();
                    stack.push(a + b);
                    break;
                }
                case SUB: {
                    int b = stack.top(); stack.pop();
                    int a = stack.top(); stack.pop();
                    stack.push(a - b);
                    break;
                }
                case JMP:
                    pc = bytecode[++pc] - 1;
                    break;
                case JZ: {
                    int addr = bytecode[++pc];
                    if (stack.top() == 0) {
                        pc = addr - 1;
                    }
                    break;
                }
                case LOAD: {
                    int index = bytecode[++pc];
                    stack.push(memory[index]);
                    break;
                }
                case STORE: {
                    int index = bytecode[++pc];
                    memory[index] = stack.top();
                    stack.pop();
                    break;
                }
                case HALT:
                    return;
                default:
                    break;
            }
            pc++;
        }
    }

    void setMemory(int index, int value) {
        if (index >= memory.size()) {
            memory.resize(index + 1);
        }
        memory[index] = value;
    }

    int getMemory(int index) const {
        return memory.at(index);
    }

private:
    std::vector<int> memory;
};

int main() {
    VirtualMachine vm;
    std::vector<int> program = {
        PUSH, 4,
        PUSH, 6,
        ADD,
        STORE, 0,
        LOAD, 0,
        PUSH, 2,
        SUB,
        JZ, 14,
        PUSH, 1,
        ADD,
        STORE, 0,
        JMP, 4,
        HALT
    };

    vm.execute(program);
    std::cout << "Memory[0]: " << vm.getMemory(0) << std::endl;

    return 0;
}