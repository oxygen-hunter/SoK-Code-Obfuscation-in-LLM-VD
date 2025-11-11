#include <stack>
#include <vector>
#include <iostream>

enum OpCode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT };

class VM {
public:
    void Run(const std::vector<int>& bytecode) {
        pc = 0;
        stack.clear();
        while (pc < bytecode.size()) {
            switch (bytecode[pc]) {
                case PUSH:
                    stack.push(bytecode[++pc]);
                    break;
                case POP:
                    stack.pop();
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
                case JZ:
                    if (stack.top() == 0) {
                        pc = bytecode[++pc] - 1;
                    } else {
                        ++pc;
                    }
                    stack.pop();
                    break;
                case LOAD:
                    stack.push(memory[bytecode[++pc]]);
                    break;
                case STORE:
                    memory[bytecode[++pc]] = stack.top();
                    stack.pop();
                    break;
                case CALL:
                    stack.push(pc + 1);
                    pc = bytecode[++pc] - 1;
                    break;
                case RET:
                    pc = stack.top();
                    stack.pop();
                    break;
                case HALT:
                    return;
                default:
                    throw std::runtime_error("Unknown opcode");
            }
            ++pc;
        }
    }

private:
    std::stack<int> stack;
    std::vector<int> memory = std::vector<int>(256, 0);
    int pc = 0;
};

int main() {
    VM vm;
    std::vector<int> program = {
        PUSH, 10,  // Initialize m_ucCount
        PUSH, 4,
        ADD,
        STORE, 0,  // Store in memory[0]
        LOAD, 0,
        PUSH, 4,
        JZ, 18,
        CALL, 12,
        JMP, 6,
        RET,
        PUSH, 0xffd0,
        STORE, 1,
        PUSH, 1000,
        STORE, 2,
        HALT
    };
    vm.Run(program);
    return 0;
}