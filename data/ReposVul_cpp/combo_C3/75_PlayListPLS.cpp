#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>

enum OpCode {
  PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
};

class VM {
public:
  VM(const std::vector<int>& program) : program(program), pc(0) {}

  void run() {
    while (pc < program.size()) {
      switch (static_cast<OpCode>(program[pc])) {
        case PUSH: stack.push_back(program[++pc]); break;
        case POP: stack.pop_back(); break;
        case ADD: binaryOp([](int a, int b) { return a + b; }); break;
        case SUB: binaryOp([](int a, int b) { return a - b; }); break;
        case JMP: pc = program[++pc] - 1; break;
        case JZ: if (stack.back() == 0) pc = program[++pc] - 1; else ++pc; stack.pop_back(); break;
        case LOAD: stack.push_back(memory[program[++pc]]); break;
        case STORE: memory[program[++pc]] = stack.back(); stack.pop_back(); break;
        case CALL: callStack.push_back(pc); pc = program[++pc] - 1; break;
        case RET: pc = callStack.back(); callStack.pop_back(); break;
        case HALT: return;
      }
      ++pc;
    }
  }

  int top() { return stack.back(); }

private:
  std::vector<int> program;
  std::vector<int> stack;
  std::map<int, int> memory;
  std::vector<int> callStack;
  int pc;

  void binaryOp(const std::function<int(int, int)>& op) {
    int b = stack.back(); stack.pop_back();
    int a = stack.back(); stack.pop_back();
    stack.push_back(op(a, b));
  }
};

int main() {
  std::vector<int> program = {
    PUSH, 5,    // PUSH 5
    PUSH, 10,   // PUSH 10
    ADD,        // ADD
    PUSH, 3,    // PUSH 3
    SUB,        // SUB
    HALT        // HALT
  };

  VM vm(program);
  vm.run();
  std::cout << vm.top() << std::endl;
  return 0;
}