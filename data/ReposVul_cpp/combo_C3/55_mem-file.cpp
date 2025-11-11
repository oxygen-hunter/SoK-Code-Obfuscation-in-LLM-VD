#include <iostream>
#include <vector>
#include <cstring>
#include <unordered_map>
#include <cstdlib>
#include <cassert>

namespace HPHP {

class VirtualMachine {
public:
    enum Instruction {
        PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT
    };

    VirtualMachine(const std::vector<int>& program)
        : m_program(program), m_pc(0), m_running(true) {}

    void run() {
        while (m_running) {
            execute(m_program[m_pc]);
        }
    }

private:
    std::vector<int> m_program;
    std::vector<int> m_stack;
    std::unordered_map<int, int> m_memory;
    int m_pc;
    bool m_running;

    void execute(int instruction) {
        switch (instruction) {
            case PUSH:
                m_stack.push_back(m_program[++m_pc]);
                break;
            case POP:
                m_stack.pop_back();
                break;
            case ADD: {
                int b = m_stack.back(); m_stack.pop_back();
                int a = m_stack.back(); m_stack.pop_back();
                m_stack.push_back(a + b);
                break;
            }
            case SUB: {
                int b = m_stack.back(); m_stack.pop_back();
                int a = m_stack.back(); m_stack.pop_back();
                m_stack.push_back(a - b);
                break;
            }
            case JMP:
                m_pc = m_program[++m_pc] - 1;
                break;
            case JZ: {
                int target = m_program[++m_pc];
                if (m_stack.back() == 0) {
                    m_pc = target - 1;
                }
                break;
            }
            case LOAD:
                m_stack.push_back(m_memory[m_program[++m_pc]]);
                break;
            case STORE:
                m_memory[m_program[++m_pc]] = m_stack.back();
                m_stack.pop_back();
                break;
            case HALT:
                m_running = false;
                break;
            default:
                std::cerr << "Unknown instruction: " << instruction << std::endl;
                m_running = false;
                break;
        }
        m_pc++;
    }
};

class MemFile {
public:
    MemFile(const char* data, int64_t len)
        : m_data(nullptr), m_len(len), m_cursor(0), m_malloced(true) {
        m_data = (char*)malloc(len + 1);
        if (m_data && len) {
            memcpy(m_data, data, len);
        }
        m_data[len] = '\0';
    }

    ~MemFile() { close(); }

    bool open(const char* filename) {
        // Simulate file open logic in VM
        std::vector<int> program = {
            VirtualMachine::PUSH, 0, // Placeholder for filename
            VirtualMachine::LOAD,
            VirtualMachine::STORE, 0,
            VirtualMachine::HALT
        };
        VirtualMachine vm(program);
        vm.run();
        return true;
    }

    bool close() {
        // Simulate close logic in VM
        std::vector<int> program = {
            VirtualMachine::PUSH, 0, // Placeholder for data
            VirtualMachine::LOAD,
            VirtualMachine::STORE, 0,
            VirtualMachine::HALT
        };
        VirtualMachine vm(program);
        vm.run();
        return true;
    }

private:
    char* m_data;
    int64_t m_len;
    int64_t m_cursor;
    bool m_malloced;
};

} // namespace HPHP