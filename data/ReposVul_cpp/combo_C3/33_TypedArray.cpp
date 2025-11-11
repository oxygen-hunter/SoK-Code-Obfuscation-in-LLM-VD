#include <iostream>
#include <vector>
#include <stack>
#include <functional>
#include <unordered_map>

enum Instruction {
    PUSH,
    POP,
    ADD,
    SUB,
    JMP,
    JZ,
    LOAD,
    STORE,
    CALL,
    RET
};

class VM {
public:
    VM(std::vector<int> program)
        : m_program(std::move(program))
    {
    }

    void run()
    {
        while (m_pc < m_program.size()) {
            auto instruction = static_cast<Instruction>(m_program[m_pc]);
            switch (instruction) {
            case PUSH:
                m_stack.push(m_program[++m_pc]);
                break;
            case POP:
                m_stack.pop();
                break;
            case ADD: {
                auto a = m_stack.top();
                m_stack.pop();
                auto b = m_stack.top();
                m_stack.pop();
                m_stack.push(a + b);
                break;
            }
            case SUB: {
                auto a = m_stack.top();
                m_stack.pop();
                auto b = m_stack.top();
                m_stack.pop();
                m_stack.push(a - b);
                break;
            }
            case JMP:
                m_pc = m_program[++m_pc] - 1;
                break;
            case JZ: {
                auto a = m_stack.top();
                m_stack.pop();
                if (a == 0)
                    m_pc = m_program[++m_pc] - 1;
                else
                    ++m_pc;
                break;
            }
            case LOAD: {
                auto addr = m_program[++m_pc];
                m_stack.push(m_memory[addr]);
                break;
            }
            case STORE: {
                auto addr = m_program[++m_pc];
                m_memory[addr] = m_stack.top();
                m_stack.pop();
                break;
            }
            case CALL: {
                m_stack.push(m_pc + 1);
                m_pc = m_program[++m_pc] - 1;
                break;
            }
            case RET:
                m_pc = m_stack.top();
                m_stack.pop();
                break;
            default:
                std::cerr << "Unknown instruction" << std::endl;
                return;
            }
            ++m_pc;
        }
    }

private:
    std::vector<int> m_program;
    std::stack<int> m_stack;
    std::unordered_map<int, int> m_memory;
    size_t m_pc { 0 };
};

// Original function logic is translated into bytecode
std::vector<int> compile_initialize_typed_array()
{
    return {
        PUSH, 100, // element_size
        LOAD, 0,   // byte_offset
        PUSH, 0,
        ADD,
        LOAD, 1,   // length
        PUSH, 0,
        JZ, 16,    // if length is undefined, jump to 16
        LOAD, 2,   // buffer_byte_length
        PUSH, 100,
        SUB,
        PUSH, 0,
        JZ, 20,    // if offset > buffer_byte_length, throw exception
        RET,
        // Additional logic for length defined
        LOAD, 3,   // new_length
        PUSH, 100,
        SUB,
        RET
    };
}

int main()
{
    VM vm(compile_initialize_typed_array());
    vm.run();
    return 0;
}