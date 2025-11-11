#include <proxygen/lib/http/codec/compress/HeaderTable.h>
#include <glog/logging.h>
#include <vector>
#include <map>
#include <list>

using std::list;
using std::pair;
using std::string;

namespace proxygen {

enum Instruction {
  PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
};

class VM {
public:
  VM(const std::vector<int>& program) : program(program), pc(0) {}

  void run() {
    while (pc < program.size()) {
      int instr = program[pc++];
      switch (instr) {
        case PUSH:
          stack.push_back(program[pc++]);
          break;
        case POP:
          stack.pop_back();
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
          pc = program[pc];
          break;
        case JZ:
          if (stack.back() == 0) pc = program[pc];
          else ++pc;
          stack.pop_back();
          break;
        case LOAD:
          stack.push_back(memory[program[pc++]]);
          break;
        case STORE:
          memory[program[pc++]] = stack.back();
          stack.pop_back();
          break;
        case CALL:
          call_stack.push_back(pc + 1);
          pc = program[pc];
          break;
        case RET:
          pc = call_stack.back();
          call_stack.pop_back();
          break;
        case HALT:
          return;
      }
    }
  }
  
  void setMemory(int index, int value) {
    memory[index] = value;
  }

  int getMemory(int index) {
    return memory[index];
  }

private:
  std::vector<int> program;
  std::vector<int> stack;
  std::vector<int> call_stack;
  std::map<int, int> memory;
  int pc;
};

class HeaderTable {
public:
  void init(uint32_t capacityVal) {
    std::vector<int> program = {
      PUSH, 0, STORE, 0,   // bytes_ = 0
      PUSH, 0, STORE, 1,   // size_ = 0
      PUSH, 0, STORE, 2,   // head_ = 0
      PUSH, capacityVal, STORE, 3,   // capacity_ = capacityVal
      PUSH, (capacityVal >> 5) + 1, STORE, 4,  // length
      CALL, 100,  // table_.assign
      CALL, 101,  // names_.clear()
      HALT,
      100,  // table_.assign
      RET,
      101,  // names_.clear()
      RET
    };
    VM vm(program);
    vm.run();
    capacity_ = vm.getMemory(3);
  }

  bool add(const HPACKHeader& header) {
    std::vector<int> program = {
      // if (bytes_ + header.bytes() > capacity_)
      LOAD, 0, PUSH, header.bytes(), ADD, LOAD, 3, SUB, JZ, 10, 
      CALL, 102, // evict(header.bytes())
      HALT,
      // if (bytes_ + header.bytes() > capacity_)
      LOAD, 0, PUSH, header.bytes(), ADD, LOAD, 3, SUB, JZ, 20,
      PUSH, 0, RET, // return false
      20,
      // if (size_ > 0)
      LOAD, 1, JZ, 30, 
      CALL, 103, // head_ = next(head_)
      30,
      CALL, 104, // table_[head_] = header
      CALL, 105, // names_[header.name].push_back(head_)
      // bytes_ += header.bytes()
      LOAD, 0, PUSH, header.bytes(), ADD, STORE, 0,
      // ++size_
      LOAD, 1, PUSH, 1, ADD, STORE, 1,
      PUSH, 1, RET, // return true
      102, // evict
      RET,
      103, // next(head_)
      RET,
      104, // table_[head_] = header
      RET,
      105, // names_[header.name].push_back(head_)
      RET
    };
    VM vm(program);
    vm.run();
    return vm.getMemory(0);
  }

private:
  uint32_t bytes_;
  uint32_t size_;
  uint32_t head_;
  uint32_t capacity_;
  std::vector<HPACKHeader> table_;
  std::map<string, list<uint32_t>> names_;
};

}