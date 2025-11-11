#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <stdexcept>

namespace tnt
{
    enum InstructionSet {
        PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, PRINT, HALT
    };

    class VM {
    public:
        VM() : pc(0) {}

        void execute(const std::vector<int>& bytecode) {
            while (pc < bytecode.size()) {
                int instr = bytecode[pc++];
                switch (instr) {
                    case PUSH:
                        stack.push_back(bytecode[pc++]);
                        break;
                    case POP:
                        if (!stack.empty()) stack.pop_back();
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
                        pc = bytecode[pc];
                        break;
                    case JZ:
                        if (stack.back() == 0) pc = bytecode[pc];
                        else pc++;
                        break;
                    case LOAD:
                        stack.push_back(bytecode[pc++]);
                        break;
                    case STORE:
                        bytecode[pc++] = stack.back();
                        stack.pop_back();
                        break;
                    case PRINT:
                        std::cout << stack.back() << std::endl;
                        break;
                    case HALT:
                        return;
                    default:
                        throw std::runtime_error("Unknown instruction");
                }
            }
        }

    private:
        std::vector<int> stack;
        int pc;
    };

    std::string toHex(char ch) {
        const static char hex[] = "0123456789abcdef";
        return std::string("\\x") + hex[(ch >> 4) & 0xf] + hex[ch & 0xf];
    }

    std::string chartoprint(char ch) {
        if (std::isprint(ch)) {
            return std::string(1, '\'') + ch + '\'';
        } else {
            return std::string("'") + toHex(ch) + "'";
        }
    }

    void executeParser(VM& vm, const std::string& input) {
        std::vector<int> bytecode = {
            LOAD, 33, JMP, 23,   // if (ch >= 33) -> jump to add logic
            LOAD, 126, JMP, 29,  // if (ch <= 126) -> jump to add logic
            PRINT,               // print invalid character
            HALT,                // end
            PUSH, 1, ADD,        // add logic
            HALT                 // end
        };

        for (char ch : input) {
            vm.execute(bytecode);
            std::cout << "Processed: " << chartoprint(ch) << std::endl;
        }
    }
}

int main() {
    tnt::VM vm;
    std::string input = "Example Input";
    tnt::executeParser(vm, input);
    return 0;
}