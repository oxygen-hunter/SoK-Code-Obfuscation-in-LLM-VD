#include <iostream>
#include <vector>
#include <stack>
#include <map>

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
  RET,
  HALT
};

class VM {
public:
  VM(std::vector<int> instructions) : instructions(instructions), pc(0) {}

  void run() {
    while (pc < instructions.size()) {
      int opcode = instructions[pc++];
      execute(opcode);
    }
  }

private:
  std::vector<int> instructions;
  std::stack<int> stack;
  std::map<int, int> memory;
  int pc;

  void execute(int opcode) {
    switch (opcode) {
    case PUSH:
      stack.push(instructions[pc++]);
      break;
    case POP:
      stack.pop();
      break;
    case ADD: {
      int b = stack.top(); stack.pop();
      int a = stack.top(); stack.pop();
      stack.push(a + b);
      break;
    }
    case SUB: {
      int b = stack.top(); stack.pop();
      int a = stack.top(); stack.pop();
      stack.push(a - b);
      break;
    }
    case JMP:
      pc = instructions[pc];
      break;
    case JZ: {
      int address = instructions[pc++];
      if (stack.top() == 0) pc = address;
      break;
    }
    case LOAD:
      stack.push(memory[instructions[pc++]]);
      break;
    case STORE:
      memory[instructions[pc++]] = stack.top();
      stack.pop();
      break;
    case CALL:
      memory[instructions[pc++]] = pc;
      pc = instructions[pc];
      break;
    case RET:
      pc = memory[instructions[pc++]];
      break;
    case HALT:
      pc = instructions.size();
      break;
    default:
      std::cerr << "Unknown instruction: " << opcode << std::endl;
      break;
    }
  }
};

int main() {
  std::vector<int> bytecode = {
    PUSH, 5,  // Load 5
    PUSH, 3,  // Load 3
    ADD,      // Add them
    PUSH, 2,  // Load 2
    SUB,      // Subtract result from sum
    HALT      // End of program
  };

  VM vm(bytecode);
  vm.run();

  return 0;
}