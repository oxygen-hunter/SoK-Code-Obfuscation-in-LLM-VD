#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cassert>
#include <cstdlib>

namespace HPHP {

enum Instruction {
  PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
};

class VM {
public:
  VM(const std::vector<int>& program) : program(program), pc(0) {}

  void run() {
    while (pc < program.size()) {
      int instruction = program[pc++];
      switch (instruction) {
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
        case JZ: {
          int addr = program[pc++];
          if (stack.back() == 0) {
            pc = addr;
          }
          break;
        }
        case LOAD:
          stack.push_back(memory[program[pc++]]);
          break;
        case STORE:
          memory[program[pc++]] = stack.back();
          break;
        case CALL: {
          int addr = program[pc++];
          return_addresses.push_back(pc);
          pc = addr;
          break;
        }
        case RET:
          pc = return_addresses.back();
          return_addresses.pop_back();
          break;
        default:
          std::cerr << "Unknown instruction: " << instruction << std::endl;
          exit(1);
      }
    }
  }

private:
  std::vector<int> program;
  std::vector<int> stack;
  std::vector<int> memory = std::vector<int>(1024, 0);
  std::vector<int> return_addresses;
  size_t pc;
};

int main() {
  std::vector<int> program = {
    PUSH, 42,
    PUSH, 23,
    ADD,
    PUSH, 1,
    SUB,
    JZ, 15,
    PUSH, 99,
    STORE, 0,
    LOAD, 0,
    RET
  };

  VM vm(program);
  vm.run();
  return 0;
}

}