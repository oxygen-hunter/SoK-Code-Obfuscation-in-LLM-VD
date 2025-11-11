#include <stack>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <type_traits>

enum Instruction {
  PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
};

class VM {
public:
  void execute(const std::vector<int>& program) {
    std::stack<int> stack;
    size_t pc = 0;
    while (pc < program.size()) {
      switch (program[pc]) {
        case PUSH:
          stack.push(program[++pc]);
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
          pc = program[++pc];
          continue;
        case JZ: {
          int address = program[++pc];
          if (stack.top() == 0) {
            pc = address;
            continue;
          }
          break;
        }
        case LOAD: {
          int address = program[++pc];
          stack.push(memory[address]);
          break;
        }
        case STORE: {
          int address = program[++pc];
          memory[address] = stack.top();
          stack.pop();
          break;
        }
        case CALL: {
          int address = program[++pc];
          callStack.push(pc);
          pc = address;
          continue;
        }
        case RET:
          pc = callStack.top();
          callStack.pop();
          break;
        default:
          std::cerr << "Unknown instruction " << program[pc] << std::endl;
          return;
      }
      pc++;
    }
  }

private:
  std::unordered_map<int, int> memory;
  std::stack<size_t> callStack;
};

int main() {
  std::vector<int> bytecode = {
    PUSH, 5, PUSH, 10, ADD, STORE, 0,
    LOAD, 0, PUSH, 3, SUB, JZ, 18,
    PUSH, 1, ADD, STORE, 0, JMP, 6,
    LOAD, 0, RET
  };
  
  VM vm;
  vm.execute(bytecode);
  
  return 0;
}