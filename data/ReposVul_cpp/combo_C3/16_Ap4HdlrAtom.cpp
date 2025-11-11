#include "Ap4HdlrAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

// VM Instruction Set
enum Instruction {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
};

// Simple VM structure
struct VM {
    std::vector<int> stack;
    std::vector<int> memory;
    std::vector<int> instructions;
    int pc = 0;

    void run() {
        while (pc < instructions.size()) {
            switch (instructions[pc++]) {
                case PUSH: stack.push_back(instructions[pc++]); break;
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
                case JMP: pc = instructions[pc]; break;
                case JZ: if (stack.back() == 0) pc = instructions[pc]; else pc++; break;
                case LOAD: stack.push_back(memory[instructions[pc++]]); break;
                case STORE: memory[instructions[pc++]] = stack.back(); stack.pop_back(); break;
                case CALL: pc = instructions[pc]; break;
                case RET: return;
                case HALT: return;
            }
        }
    }
};

AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_HdlrAtom)

AP4_HdlrAtom* AP4_HdlrAtom::Create(AP4_Size size, AP4_ByteStream& stream) {
    VM vm;
    vm.instructions = {
        PUSH, size,
        PUSH, reinterpret_cast<int>(&stream),
        CALL, 20,
        LOAD, 0,
        JZ, 30,
        PUSH, 0,
        JZ, 30,
        CALL, 40,
        RET,
        HALT,
        // Function 20
        LOAD, 1,
        LOAD, 2,
        CALL, 50,
        RET,
        // Function 30
        PUSH, 0,
        RET,
        // Function 40
        PUSH, 0,
        RET,
        // Function 50
        CALL, 60,
        RET,
        // Function 60
        PUSH, 0,
        RET
    };
    vm.memory.assign(10, 0);
    vm.run();
    return reinterpret_cast<AP4_HdlrAtom*>(vm.stack.back());
}

AP4_HdlrAtom::AP4_HdlrAtom(AP4_Atom::Type hdlr_type, const char* hdlr_name) :
    AP4_Atom(AP4_ATOM_TYPE_HDLR, AP4_FULL_ATOM_HEADER_SIZE, 0, 0),
    m_HandlerType(hdlr_type),
    m_HandlerName(hdlr_name)
{
    VM vm;
    vm.instructions = {
        PUSH, 20,
        PUSH, m_HandlerName.GetLength() + 1,
        ADD,
        STORE, 0,
        PUSH, 0,
        STORE, 1,
        STORE, 2,
        RET,
        HALT
    };
    vm.memory.assign(3, 0);
    vm.run();
    m_Size32 += vm.memory[0];
    m_Reserved[0] = m_Reserved[1] = m_Reserved[2] = vm.memory[1];
}

AP4_HdlrAtom::AP4_HdlrAtom(AP4_UI32 size, AP4_UI08 version, AP4_UI32 flags, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_HDLR, size, version, flags)
{
    VM vm;
    vm.instructions = {
        PUSH, reinterpret_cast<int>(&stream),
        STORE, 0,
        CALL, 100,
        RET,
        HALT,
        // Function 100
        LOAD, 0,
        CALL, 110,
        LOAD, 0,
        CALL, 120,
        LOAD, 0,
        CALL, 130,
        LOAD, 0,
        CALL, 140,
        LOAD, 0,
        CALL, 150,
        RET,
        // Function 110
        PUSH, reinterpret_cast<int>(&m_HandlerType),
        STORE, 1,
        RET,
        // Function 120
        PUSH, reinterpret_cast<int>(&m_Reserved[0]),
        STORE, 2,
        RET,
        // Function 130
        PUSH, reinterpret_cast<int>(&m_Reserved[1]),
        STORE, 3,
        RET,
        // Function 140
        PUSH, reinterpret_cast<int>(&m_Reserved[2]),
        STORE, 4,
        RET,
        // Function 150
        PUSH, size,
        PUSH, AP4_FULL_ATOM_HEADER_SIZE + 20,
        SUB,
        STORE, 5,
        LOAD, 5,
        JZ, 200,
        PUSH, 0,
        RET,
        // Function 200
        LOAD, 5,
        PUSH, 1,
        ADD,
        CALL, 210,
        RET,
        // Function 210
        PUSH, 1,
        JZ, 220,
        PUSH, 0,
        RET,
        // Function 220
        PUSH, 0,
        RET
    };
    vm.memory.assign(6, 0);
    vm.run();
}

AP4_Result AP4_HdlrAtom::WriteFields(AP4_ByteStream& stream) {
    VM vm;
    vm.instructions = {
        CALL, 300,
        RET,
        HALT,
        // Function 300
        PUSH, reinterpret_cast<int>(&stream),
        STORE, 0,
        CALL, 310,
        LOAD, 0,
        CALL, 320,
        LOAD, 0,
        CALL, 330,
        LOAD, 0,
        CALL, 340,
        LOAD, 0,
        CALL, 350,
        RET,
        // Function 310
        PUSH, 0,
        STORE, 1,
        RET,
        // Function 320
        PUSH, m_HandlerType,
        STORE, 2,
        RET,
        // Function 330
        PUSH, m_Reserved[0],
        STORE, 3,
        RET,
        // Function 340
        PUSH, m_Reserved[1],
        STORE, 4,
        RET,
        // Function 350
        PUSH, m_Reserved[2],
        STORE, 5,
        RET
    };
    vm.memory.assign(6, 0);
    vm.run();
    return AP4_SUCCESS;
}

AP4_Result AP4_HdlrAtom::InspectFields(AP4_AtomInspector& inspector) {
    VM vm;
    vm.instructions = {
        PUSH, reinterpret_cast<int>(&inspector),
        STORE, 0,
        CALL, 400,
        RET,
        HALT,
        // Function 400
        PUSH, reinterpret_cast<int>(&m_HandlerType),
        STORE, 1,
        PUSH, reinterpret_cast<int>(&m_HandlerName),
        STORE, 2,
        RET
    };
    vm.memory.assign(3, 0);
    vm.run();
    return AP4_SUCCESS;
}