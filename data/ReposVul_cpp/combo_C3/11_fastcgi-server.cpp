#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <chrono>
#include <memory>
#include <functional>
#include <system_error>
#include <cassert>

namespace HPHP {

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
  VM() : pc(0), running(true) {}

  void run(const std::vector<int>& program) {
    while (running) {
      dispatch(program[pc]);
    }
  }

private:
  std::vector<int> stack;
  std::unordered_map<int, int> memory;
  int pc;
  bool running;

  void dispatch(int instruction) {
    switch (instruction) {
      case PUSH: stack.push_back(next()); break;
      case POP: stack.pop_back(); break;
      case ADD: binaryOp(std::plus<int>()); break;
      case SUB: binaryOp(std::minus<int>()); break;
      case JMP: pc = next(); break;
      case JZ: zeroJump(); break;
      case LOAD: load(); break;
      case STORE: store(); break;
      case CALL: call(); break;
      case RET: ret(); break;
      case HALT: running = false; break;
      default: throw std::runtime_error("Unknown instruction");
    }
  }

  int next() {
    return stack[pc++];
  }

  void binaryOp(const std::function<int(int, int)>& op) {
    int b = stack.back(); stack.pop_back();
    int a = stack.back(); stack.pop_back();
    stack.push_back(op(a, b));
  }

  void zeroJump() {
    if (stack.back() == 0) {
      pc = next();
    } else {
      pc++;
    }
  }

  void load() {
    int addr = next();
    stack.push_back(memory[addr]);
  }

  void store() {
    int addr = next();
    memory[addr] = stack.back();
    stack.pop_back();
  }

  void call() {
    int addr = next();
    stack.push_back(pc);
    pc = addr;
  }

  void ret() {
    pc = stack.back();
    stack.pop_back();
  }
};

bool FastCGIAcceptor_canAccept(int cons, int limit) {
  VM vm;
  std::vector<int> program = {
    PUSH, cons,
    PUSH, limit,
    SUB,
    JZ, 11,
    PUSH, 1,
    JMP, 13,
    PUSH, 0,
    HALT
  };
  vm.run(program);
  return vm.stack.back() != 0;
}

void FastCGIServer_start(bool useFileSocket, std::string address, int port) {
  VM vm;
  std::vector<int> program = {
    PUSH, useFileSocket,
    JZ, 7,
    PUSH, 0,
    JMP, 12,
    PUSH, 1,
    HALT
  };
  vm.run(program);
  if (vm.stack.back()) {
    std::cout << "Using file socket with address: " << address << std::endl;
  } else {
    std::cout << "Using network socket with address: " << address << " and port: " << port << std::endl;
  }
}

} // namespace HPHP