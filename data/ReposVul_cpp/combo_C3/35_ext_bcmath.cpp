#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/bcmath/bcmath.h"
#include "hphp/runtime/base/ini-setting.h"
#include <folly/ScopeGuard.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define PUSH 0
#define POP 1
#define ADD 2
#define SUB 3
#define MUL 4
#define DIV 5
#define MOD 6
#define LOAD 7
#define STORE 8
#define JMP 9
#define JZ 10
#define CALL 11
#define RET 12

struct VM {
  std::vector<int64_t> stack;
  std::unordered_map<std::string, int64_t> memory;
  std::vector<int64_t> program;
  int pc = 0;

  void run() {
    while (pc < program.size()) {
      int64_t opcode = program[pc++];
      switch (opcode) {
        case PUSH:
          stack.push_back(program[pc++]);
          break;
        case POP:
          stack.pop_back();
          break;
        case ADD: {
          int64_t b = stack.back(); stack.pop_back();
          int64_t a = stack.back(); stack.pop_back();
          stack.push_back(a + b);
          break;
        }
        case SUB: {
          int64_t b = stack.back(); stack.pop_back();
          int64_t a = stack.back(); stack.pop_back();
          stack.push_back(a - b);
          break;
        }
        case MUL: {
          int64_t b = stack.back(); stack.pop_back();
          int64_t a = stack.back(); stack.pop_back();
          stack.push_back(a * b);
          break;
        }
        case DIV: {
          int64_t b = stack.back(); stack.pop_back();
          int64_t a = stack.back(); stack.pop_back();
          stack.push_back(a / b);
          break;
        }
        case MOD: {
          int64_t b = stack.back(); stack.pop_back();
          int64_t a = stack.back(); stack.pop_back();
          stack.push_back(a % b);
          break;
        }
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
        case JZ: {
          int64_t addr = program[pc++];
          if (stack.back() == 0) pc = addr;
          break;
        }
        case CALL: {
          int64_t addr = program[pc++];
          stack.push_back(pc);
          pc = addr;
          break;
        }
        case RET:
          pc = stack.back();
          stack.pop_back();
          break;
      }
    }
  }
};

///////////////////////////////////////////////////////////////////////////////

void run_bcscale(VM &vm) {
  vm.program = {
    PUSH, -1,
    CALL, 4,
    STORE, "bc_precision",
    RET,
    // adjust_scale function
    LOAD, "bc_precision",
    PUSH, 0,
    JZ, 12,
    LOAD, "bc_precision",
    RETURN,
    // end of adjust_scale
  };
  vm.run();
}

void run_bcadd(VM &vm) {
  vm.program = {
    PUSH, -1,
    CALL, 4,
    LOAD, "first",
    LOAD, "second",
    ADD,
    STORE, "result",
    RET
  };
  vm.run();
}

void run_bcsub(VM &vm) {
  vm.program = {
    PUSH, -1,
    CALL, 4,
    LOAD, "first",
    LOAD, "second",
    SUB,
    STORE, "result",
    RET
  };
  vm.run();
}

void run_bcmul(VM &vm) {
  vm.program = {
    PUSH, -1,
    CALL, 4,
    LOAD, "first",
    LOAD, "second",
    MUL,
    STORE, "result",
    RET
  };
  vm.run();
}

void run_bcdiv(VM &vm) {
  vm.program = {
    PUSH, -1,
    CALL, 4,
    LOAD, "first",
    LOAD, "second",
    DIV,
    STORE, "result",
    RET
  };
  vm.run();
}

void run_bcmod(VM &vm) {
  vm.program = {
    LOAD, "first",
    LOAD, "second",
    MOD,
    STORE, "result",
    RET
  };
  vm.run();
}

void run_bcpow(VM &vm) {
  vm.program = {
    PUSH, -1,
    CALL, 4,
    LOAD, "first",
    LOAD, "second",
    CALL, 25, // assuming power function
    STORE, "result",
    RET
  };
  vm.run();
}

void run_bcpowmod(VM &vm) {
  vm.program = {
    PUSH, -1,
    CALL, 4,
    LOAD, "first",
    LOAD, "second",
    LOAD, "modulus",
    CALL, 35, // assuming power mod function
    STORE, "result",
    RET
  };
  vm.run();
}

void run_bcsqrt(VM &vm) {
  vm.program = {
    PUSH, -1,
    CALL, 4,
    LOAD, "operand",
    CALL, 45, // assuming sqrt function
    STORE, "result",
    RET
  };
  vm.run();
}

///////////////////////////////////////////////////////////////////////////////

struct bcmathExtension final : Extension {
  bcmathExtension() : Extension("bcmath", NO_EXTENSION_VERSION_YET) {}
  void moduleInit() override {
    HHVM_FE(bcscale);
    HHVM_FE(bcadd);
    HHVM_FE(bcsub);
    HHVM_FE(bccomp);
    HHVM_FE(bcmul);
    HHVM_FE(bcdiv);
    HHVM_FE(bcmod);
    HHVM_FE(bcpow);
    HHVM_FE(bcpowmod);
    HHVM_FE(bcsqrt);
    loadSystemlib();
  }

  void threadInit() override {
    IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                     "bcmath.scale", "0",
                     &BCG(bc_precision));
  }

} s_bcmath_extension;

///////////////////////////////////////////////////////////////////////////////

extern "C" {
  struct BCMathGlobals *get_bcmath_globals() {
    return &HPHP::s_globals.get()->data;
  }
}

///////////////////////////////////////////////////////////////////////////////
}