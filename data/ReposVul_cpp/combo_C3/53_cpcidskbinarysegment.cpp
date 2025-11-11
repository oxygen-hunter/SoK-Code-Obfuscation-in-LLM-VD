#include "segment/cpcidskbinarysegment.h"
#include "segment/cpcidsksegment.h"
#include "core/pcidsk_utils.h"
#include "pcidsk_exception.h"
#include "core/pcidsk_utils.h"

#include <limits>
#include <vector>
#include <string>
#include <cassert>
#include <cstring>

using namespace PCIDSK;

class SimpleVM {
public:
    enum Instruction {
        NOP, PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
    };

    SimpleVM() : pc(0) {}

    void execute(const std::vector<int>& bytecode) {
        while (pc < bytecode.size()) {
            switch (bytecode[pc]) {
                case NOP: pc++; break;
                case PUSH: stack.push_back(bytecode[++pc]); pc++; break;
                case POP: stack.pop_back(); pc++; break;
                case ADD: {
                    int b = stack.back(); stack.pop_back();
                    int a = stack.back(); stack.pop_back();
                    stack.push_back(a + b); pc++;
                    break;
                }
                case SUB: {
                    int b = stack.back(); stack.pop_back();
                    int a = stack.back(); stack.pop_back();
                    stack.push_back(a - b); pc++;
                    break;
                }
                case JMP: pc += bytecode[++pc]; break;
                case JZ: {
                    int offset = bytecode[++pc];
                    if (stack.back() == 0) pc += offset;
                    else pc++;
                    break;
                }
                case LOAD: stack.push_back(memory[bytecode[++pc]]); pc++; break;
                case STORE: memory[bytecode[++pc]] = stack.back(); stack.pop_back(); pc++; break;
                case CALL: {
                    int addr = bytecode[++pc];
                    stack.push_back(pc + 1);
                    pc = addr;
                    break;
                }
                case RET: pc = stack.back(); stack.pop_back(); break;
                default: pc++; break;
            }
        }
    }

private:
    std::vector<int> stack;
    std::vector<int> memory;
    int pc;
};

CPCIDSKBinarySegment::CPCIDSKBinarySegment(PCIDSKFile *fileIn,
                                           int segmentIn,
                                           const char *segment_pointer,
                                           bool bLoad) :
    CPCIDSKSegment(fileIn, segmentIn, segment_pointer),
    loaded_(false), mbModified(false)
{
    SimpleVM vm;
    std::vector<int> bytecode = {
        SimpleVM::PUSH, static_cast<int>(bLoad),
        SimpleVM::JZ, 4,
        SimpleVM::CALL, 10,
        SimpleVM::RET,
        // Load function
        SimpleVM::NOP,
        SimpleVM::PUSH, static_cast<int>(data_size),
        SimpleVM::PUSH, 1024,
        SimpleVM::SUB,
        SimpleVM::PUSH, static_cast<int>(std::numeric_limits<int>::max()),
        SimpleVM::JMP, 5,
        SimpleVM::POP,
        SimpleVM::PUSH, 1,
        SimpleVM::RET
    };
    vm.execute(bytecode);
}

CPCIDSKBinarySegment::~CPCIDSKBinarySegment() {}

void CPCIDSKBinarySegment::Load() {
    SimpleVM vm;
    std::vector<int> bytecode = {
        SimpleVM::PUSH, static_cast<int>(loaded_),
        SimpleVM::JZ, 4,
        SimpleVM::RET,
        SimpleVM::PUSH, static_cast<int>(data_size),
        SimpleVM::PUSH, 1024,
        SimpleVM::SUB,
        SimpleVM::PUSH, static_cast<int>(std::numeric_limits<int>::max()),
        SimpleVM::JZ, 5,
        SimpleVM::CALL, 10,
        SimpleVM::RET,
        SimpleVM::NOP
    };
    vm.execute(bytecode);
    if (vm.execute(bytecode)) {
        seg_data.SetSize((int)(data_size - 1024));
        ReadFromFile(seg_data.buffer, 0, data_size - 1024);
        loaded_ = true;
    }
}

void CPCIDSKBinarySegment::Write(void) {
    SimpleVM vm;
    std::vector<int> bytecode = {
        SimpleVM::PUSH, static_cast<int>(loaded_),
        SimpleVM::JZ, 4,
        SimpleVM::CALL, 10,
        SimpleVM::RET,
        SimpleVM::NOP
    };
    vm.execute(bytecode);
    if (loaded_) {
        WriteToFile(seg_data.buffer, 0, seg_data.buffer_size);
        mbModified = false;
    }
}

void CPCIDSKBinarySegment::Synchronize() {
    SimpleVM vm;
    std::vector<int> bytecode = {
        SimpleVM::PUSH, static_cast<int>(mbModified),
        SimpleVM::JZ, 4,
        SimpleVM::CALL, 10,
        SimpleVM::RET,
        SimpleVM::NOP
    };
    vm.execute(bytecode);
    if (mbModified) {
        this->Write();
    }
}

void CPCIDSKBinarySegment::SetBuffer(const char* pabyBuf, unsigned int nBufSize) {
    SimpleVM vm;
    std::vector<int> bytecode = {
        SimpleVM::PUSH, static_cast<int>(nBufSize),
        SimpleVM::PUSH, 512,
        SimpleVM::DIV,
        SimpleVM::PUSH, static_cast<int>((0 == nBufSize % 512) ? 0 : 1),
        SimpleVM::ADD,
        SimpleVM::PUSH, 512,
        SimpleVM::MUL,
        SimpleVM::STORE, 0,
        SimpleVM::PUSH, 1024,
        SimpleVM::ADD,
        SimpleVM::STORE, 1,
        SimpleVM::CALL, 10,
        SimpleVM::RET,
        SimpleVM::NOP
    };
    vm.execute(bytecode);
    int nNumBlocks = nBufSize / 512 + ((0 == nBufSize % 512) ? 0 : 1);
    unsigned int nAllocBufSize = 512 * nNumBlocks;

    seg_data.SetSize((int)nAllocBufSize);
    data_size = nAllocBufSize + 1024;

    memcpy(seg_data.buffer, pabyBuf, nBufSize);

    if (nBufSize < nAllocBufSize) {
        memset(seg_data.buffer + nBufSize, 0, nAllocBufSize - nBufSize);
    }
    mbModified = true;
}