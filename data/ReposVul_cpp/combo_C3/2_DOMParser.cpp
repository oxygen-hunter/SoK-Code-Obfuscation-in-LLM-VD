#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <functional>
#include <memory>

enum Instruction {
  PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL
};

class VM {
public:
  VM() : pc(0) {}

  void execute(const std::vector<int>& bytecode) {
    while (pc < bytecode.size()) {
      dispatch(bytecode[pc++]);
    }
  }

private:
  std::stack<int> stack;
  std::map<int, int> memory;
  int pc;

  void dispatch(int instruction) {
    switch (instruction) {
      case PUSH:
        stack.push(pc++);
        break;
      case POP:
        stack.pop();
        break;
      case ADD: {
        int a = stack.top(); stack.pop();
        int b = stack.top(); stack.pop();
        stack.push(a + b);
        break;
      }
      case SUB: {
        int a = stack.top(); stack.pop();
        int b = stack.top(); stack.pop();
        stack.push(b - a);
        break;
      }
      case JMP:
        pc = stack.top();
        stack.pop();
        break;
      case JZ: {
        int addr = stack.top(); stack.pop();
        if (stack.top() == 0) {
          pc = addr;
        }
        stack.pop();
        break;
      }
      case LOAD: {
        int addr = stack.top();
        stack.pop();
        stack.push(memory[addr]);
        break;
      }
      case STORE: {
        int addr = stack.top();
        stack.pop();
        memory[addr] = stack.top();
        stack.pop();
        break;
      }
      case CALL:
        // Simulate function call
        break;
      default:
        break;
    }
  }
};

class DOMParser {
public:
  DOMParser() : vm(std::make_unique<VM>()) {}

  void ParseFromString() {
    std::vector<int> bytecode = {
      PUSH, 2, PUSH, 3, ADD, STORE, 0, LOAD, 0, PUSH, 0, JZ, 10, CALL
    };
    vm->execute(bytecode);
  }

private:
  std::unique_ptr<VM> vm;
};

int main() {
  DOMParser parser;
  parser.ParseFromString();
  return 0;
}