#include <iostream>
#include <vector>
#include <string>
#include <stack>

enum Instructions {
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

class VM {
public:
    VM(const std::vector<int>& code) : code(code), pc(0), sp(-1), running(true) {
        stack.resize(256);
    }

    void run() {
        while (running) {
            step();
        }
    }

private:
    std::vector<int> code;
    std::vector<int> stack;
    int pc;
    int sp;
    bool running;

    void step() {
        switch (code[pc++]) {
            case PUSH: {
                int val = code[pc++];
                stack[++sp] = val;
                break;
            }
            case POP: {
                sp--;
                break;
            }
            case ADD: {
                int b = stack[sp--];
                int a = stack[sp--];
                stack[++sp] = a + b;
                break;
            }
            case SUB: {
                int b = stack[sp--];
                int a = stack[sp--];
                stack[++sp] = a - b;
                break;
            }
            case JMP: {
                pc = code[pc];
                break;
            }
            case JZ: {
                int addr = code[pc++];
                if (stack[sp--] == 0) {
                    pc = addr;
                }
                break;
            }
            case LOAD: {
                int addr = code[pc++];
                stack[++sp] = stack[addr];
                break;
            }
            case STORE: {
                int addr = code[pc++];
                stack[addr] = stack[sp--];
                break;
            }
            case CALL: {
                int addr = code[pc++];
                stack[++sp] = pc;
                pc = addr;
                break;
            }
            case RET: {
                pc = stack[sp--];
                break;
            }
            case HALT: {
                running = false;
                break;
            }
            default: {
                std::cerr << "Unknown instruction at " << pc - 1 << std::endl;
                running = false;
                break;
            }
        }
    }
};

int main() {
    std::vector<int> bytecode = {
        PUSH, 5,  // Push 5 onto the stack
        PUSH, 3,  // Push 3 onto the stack
        ADD,      // Add the top two values
        HALT      // Stop the VM
    };

    VM vm(bytecode);
    vm.run();

    return 0;
}