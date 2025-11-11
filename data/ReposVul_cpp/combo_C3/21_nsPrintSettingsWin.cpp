#include "nsPrintSettingsWin.h"
#include "mozilla/ArrayUtils.h"
#include "nsCRT.h"
#include "nsDeviceContextSpecWin.h"
#include "nsPrintSettingsImpl.h"
#include "WinUtils.h"

using namespace mozilla;

enum Instructions {
  PUSH, POP, ADD, JMP, JZ, LOAD, STORE, HALT
};

struct VM {
  std::vector<int> stack;
  std::unordered_map<int, int> memory;
  int pc;
  std::vector<int> program;

  void run() {
    while (pc < program.size()) {
      switch (program[pc]) {
        case PUSH:
          stack.push_back(program[++pc]);
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
        case JMP:
          pc = program[++pc] - 1;
          break;
        case JZ: {
          int addr = program[++pc];
          if (stack.back() == 0) pc = addr - 1;
          break;
        }
        case LOAD:
          stack.push_back(memory[program[++pc]]);
          break;
        case STORE:
          memory[program[++pc]] = stack.back();
          stack.pop_back();
          break;
        case HALT:
          return;
        default:
          break;
      }
      ++pc;
    }
  }
};

void executeOriginalLogic() {
  const short kPaperSizeUnits[] = {
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
  };

  VM vm;
  vm.program = {
    PUSH, 0, LOAD, 0, HALT
  };

  vm.run();
}

int main() {
  executeOriginalLogic();
  return 0;
}