#include <iostream>
#include <vector>
#include <stack>
#include <string>
#include <map>
#include <functional>

class VM {
public:
    enum Instruction {
        PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, PRINT
    };

    void run(const std::vector<int>& program) {
        std::stack<int> stack;
        std::map<int, int> memory;
        size_t pc = 0;
        while (pc < program.size()) {
            switch (program[pc]) {
            case PUSH: {
                int value = program[++pc];
                stack.push(value);
                pc++;
                break;
            }
            case POP: {
                stack.pop();
                pc++;
                break;
            }
            case ADD: {
                int a = stack.top(); stack.pop();
                int b = stack.top(); stack.pop();
                stack.push(a + b);
                pc++;
                break;
            }
            case SUB: {
                int a = stack.top(); stack.pop();
                int b = stack.top(); stack.pop();
                stack.push(a - b);
                pc++;
                break;
            }
            case JMP: {
                pc = program[++pc];
                break;
            }
            case JZ: {
                int addr = program[++pc];
                if (stack.top() == 0) {
                    pc = addr;
                } else {
                    pc++;
                }
                break;
            }
            case LOAD: {
                int addr = program[++pc];
                stack.push(memory[addr]);
                pc++;
                break;
            }
            case STORE: {
                int addr = program[++pc];
                int value = stack.top(); stack.pop();
                memory[addr] = value;
                pc++;
                break;
            }
            case PRINT: {
                int value = stack.top();
                std::cout << "PRINT: " << value << std::endl;
                pc++;
                break;
            }
            default:
                throw std::runtime_error("Unknown instruction");
            }
        }
    }
};

int main() {
    VM vm;
    std::vector<int> program = {
        VM::PUSH, 5,
        VM::PUSH, 3,
        VM::ADD,
        VM::PRINT,
        VM::PUSH, 10,
        VM::SUB,
        VM::PRINT,
        VM::PUSH, 0,
        VM::JZ, 16,
        VM::PUSH, 1,
        VM::PRINT
    };
    vm.run(program);
    return 0;
}