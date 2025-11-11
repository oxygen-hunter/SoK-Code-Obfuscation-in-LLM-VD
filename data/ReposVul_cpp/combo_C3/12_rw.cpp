// rw.cpp - transformed into a stack-based VM

#include "pch.h"
#include "rw.h"
#include "nbtheory.h"
#include "asn.h"

#ifndef CRYPTOPP_IMPORTS

NAMESPACE_BEGIN(CryptoPP)

// VM instruction set
enum Instructions { NOP, PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, MOD, SQUARE, NEG, INC, DEC };

class VM {
public:
    VM() : pc(0) {}

    void execute(std::vector<int> &program, std::vector<Integer> &stack, std::vector<Integer> &memory) {
        Integer tmp;
        while (pc < program.size()) {
            int instr = program[pc++];
            switch (instr) {
                case NOP:
                    break;
                case PUSH:
                    stack.push_back(memory[program[pc++]]);
                    break;
                case POP:
                    stack.pop_back();
                    break;
                case ADD:
                    tmp = stack.back(); stack.pop_back();
                    stack.back() += tmp;
                    break;
                case SUB:
                    tmp = stack.back(); stack.pop_back();
                    stack.back() -= tmp;
                    break;
                case MOD:
                    tmp = stack.back(); stack.pop_back();
                    stack.back() %= tmp;
                    break;
                case SQUARE:
                    stack.back() = stack.back().Squared();
                    break;
                case NEG:
                    stack.back().Negate();
                    break;
                case LOAD:
                    stack.push_back(memory[program[pc++]]);
                    break;
                case STORE:
                    memory[program[pc++]] = stack.back();
                    stack.pop_back();
                    break;
                case JMP:
                    pc = program[pc];
                    break;
                case JZ:
                    if (stack.back() == Integer::Zero())
                        pc = program[pc];
                    else
                        pc++;
                    stack.pop_back();
                    break;
                case INC:
                    stack.back()++;
                    break;
                case DEC:
                    stack.back()--;
                    break;
                default:
                    throw std::runtime_error("Unknown instruction");
            }
        }
    }

private:
    int pc;
};

void RWFunction::BERDecode(BufferedTransformation &bt) {
    std::vector<int> program = { PUSH, 0, CALL, 1, NOP };
    std::vector<Integer> stack;
    std::vector<Integer> memory(1);
    memory[0] = Integer(bt.MaxRetrievable());
    VM vm;
    vm.execute(program, stack, memory);
    m_n.BERDecode(bt);
}

void RWFunction::DEREncode(BufferedTransformation &bt) const {
    std::vector<int> program = { PUSH, 0, CALL, 1, NOP };
    std::vector<Integer> stack;
    std::vector<Integer> memory(1);
    memory[0] = Integer(bt.MaxRetrievable());
    VM vm;
    vm.execute(program, stack, memory);
    m_n.DEREncode(bt);
}

Integer RWFunction::ApplyFunction(const Integer &in) const {
    std::vector<int> program = {
        PUSH, 0, SQUARE, MOD, 1, STORE, 2,
        LOAD, 2, MOD, 16, STORE, 3,
        LOAD, 3, PUSH, 12, SUB, JZ, 18,
        LOAD, 3, PUSH, 6, SUB, JZ, 23,
        LOAD, 3, PUSH, 14, SUB, JZ, 23,
        LOAD, 3, PUSH, 9, SUB, JZ, 32,
        LOAD, 3, PUSH, 5, SUB, JZ, 32,
        JMP, 39,
        LOAD, 2, NEG, ADD, 1, STORE, 2, JMP, 39,
        LOAD, 2, NEG, ADD, 1, INC, STORE, 2,
        LOAD, 2, PUSH, 0, STORE, 2,
        LOAD, 2
    };
    std::vector<Integer> stack;
    std::vector<Integer> memory(4);
    memory[0] = in;
    memory[1] = m_n;
    VM vm;
    vm.execute(program, stack, memory);
    return memory[2];
}

bool RWFunction::Validate(RandomNumberGenerator &rng, unsigned int level) const {
    std::vector<int> program = {
        PUSH, 0, PUSH, 1, PUSH, 8, MOD, PUSH, 5, SUB, JZ, 10, PUSH, 0
    };
    std::vector<Integer> stack;
    std::vector<Integer> memory(2);
    memory[0] = Integer::One();
    memory[1] = m_n;
    VM vm;
    vm.execute(program, stack, memory);
    return stack.back() == Integer::Zero();
}

NAMESPACE_END

#endif