#include "Ap4HdlrAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

#define PUSH 0x01
#define POP 0x02
#define ADD 0x03
#define SUB 0x04
#define JMP 0x05
#define JZ 0x06
#define LOAD 0x07
#define STORE 0x08
#define HALT 0xFF

class VM {
public:
    VM(const std::vector<int>& code) : pc(0), code(code) {}
    
    void run() {
        while (pc < code.size()) {
            int instr = code[pc++];
            switch (instr) {
                case PUSH: {
                    int value = code[pc++];
                    stack.push_back(value);
                    break;
                }
                case POP: {
                    stack.pop_back();
                    break;
                }
                case ADD: {
                    int a = stack.back(); stack.pop_back();
                    int b = stack.back(); stack.pop_back();
                    stack.push_back(a + b);
                    break;
                }
                case SUB: {
                    int a = stack.back(); stack.pop_back();
                    int b = stack.back(); stack.pop_back();
                    stack.push_back(b - a);
                    break;
                }
                case JMP: {
                    int addr = code[pc++];
                    pc = addr;
                    break;
                }
                case JZ: {
                    int addr = code[pc++];
                    if (stack.back() == 0) {
                        pc = addr;
                    }
                    stack.pop_back();
                    break;
                }
                case LOAD: {
                    int addr = code[pc++];
                    stack.push_back(memory[addr]);
                    break;
                }
                case STORE: {
                    int addr = code[pc++];
                    memory[addr] = stack.back();
                    stack.pop_back();
                    break;
                }
                case HALT: {
                    return;
                }
            }
        }
    }
    
private:
    int pc;
    std::vector<int> stack;
    std::vector<int> memory;
    std::vector<int> code;
};

AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_HdlrAtom)

AP4_HdlrAtom* AP4_HdlrAtom::Create(AP4_Size size, AP4_ByteStream& stream) {
    std::vector<int> code = {
        PUSH, size,
        LOAD, 0,
        PUSH, 0,
        ADD,
        JZ, 14,
        PUSH, 0,
        PUSH, 0,
        ADD,
        HALT
    };
    VM vm(code);
    vm.run();
    return NULL;
}

AP4_HdlrAtom::AP4_HdlrAtom(AP4_Atom::Type hdlr_type, const char* hdlr_name) :
    AP4_Atom(AP4_ATOM_TYPE_HDLR, AP4_FULL_ATOM_HEADER_SIZE, 0, 0),
    m_HandlerType(hdlr_type),
    m_HandlerName(hdlr_name)
{
    std::vector<int> code = {
        PUSH, 20,
        PUSH, m_HandlerName.GetLength(),
        ADD,
        PUSH, 1,
        ADD,
        STORE, 0,
        HALT
    };
    VM vm(code);
    vm.run();
    m_Reserved[0] = m_Reserved[1] = m_Reserved[2] = 0;
}

AP4_HdlrAtom::AP4_HdlrAtom(AP4_UI32 size, AP4_UI08 version, AP4_UI32 flags, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_HDLR, size, version, flags)
{
    std::vector<int> code = {
        LOAD, 0,
        PUSH, AP4_FULL_ATOM_HEADER_SIZE + 20,
        SUB,
        JZ, 14,
        LOAD, 0,
        PUSH, size,
        SUB,
        STORE, 0,
        HALT
    };
    VM vm(code);
    vm.run();
}

AP4_Result AP4_HdlrAtom::WriteFields(AP4_ByteStream& stream) {
    std::vector<int> code = {
        PUSH, 0,
        STORE, 0,
        HALT
    };
    VM vm(code);
    vm.run();
    return AP4_SUCCESS;
}

AP4_Result AP4_HdlrAtom::InspectFields(AP4_AtomInspector& inspector) {
    std::vector<int> code = {
        HALT
    };
    VM vm(code);
    vm.run();
    return AP4_SUCCESS;
}