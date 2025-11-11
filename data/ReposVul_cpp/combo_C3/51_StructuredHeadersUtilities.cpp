#include "StructuredHeadersUtilities.h"
#include "StructuredHeadersConstants.h"

#include "proxygen/lib/utils/Base64.h"

namespace proxygen {
namespace StructuredHeaders {

enum Instruction {
  PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL
};

class VM {
public:
  VM(const std::vector<int>& prog) : program(prog), pc(0) {}

  void run() {
    while (pc < program.size()) {
      int instr = program[pc++];
      switch (instr) {
        case PUSH: stack.push_back(program[pc++]); break;
        case POP: stack.pop_back(); break;
        case ADD: binaryOp([](int a, int b) { return a + b; }); break;
        case SUB: binaryOp([](int a, int b) { return a - b; }); break;
        case JMP: pc = program[pc]; break;
        case JZ: if (stack.back() == 0) pc = program[pc]; else pc++; stack.pop_back(); break;
        case LOAD: stack.push_back(memory[program[pc++]]); break;
        case STORE: memory[program[pc++]] = stack.back(); stack.pop_back(); break;
        case CALL: {
          int addr = program[pc++];
          (this->*functions[addr])();
          break;
        }
      }
    }
  }

private:
  void binaryOp(std::function<int(int, int)> op) {
    int b = stack.back(); stack.pop_back();
    int a = stack.back(); stack.pop_back();
    stack.push_back(op(a, b));
  }

  std::vector<int> stack;
  std::vector<int> memory;
  std::vector<int> program;
  int pc;

  using Function = void (VM::*)();
  std::vector<Function> functions = {
    &VM::isLcAlpha,
    &VM::isValidIdentifierChar,
    &VM::isValidEncodedBinaryContentChar,
    &VM::isValidStringChar,
    &VM::isValidIdentifier,
    &VM::isValidString,
    &VM::isValidEncodedBinaryContent,
    &VM::itemTypeMatchesContent,
    &VM::decodeBase64,
    &VM::encodeBase64
  };

  void isLcAlpha() {
    int c = stack.back(); stack.pop_back();
    stack.push_back(c >= 0x61 && c <= 0x7A);
  }

  void isValidIdentifierChar() {
    int c = stack.back(); stack.pop_back();
    stack.push_back(isLcAlpha(c) || std::isdigit(c) || c == '_' || c == '-' || c == '*' || c == '/');
  }

  void isValidEncodedBinaryContentChar() {
    int c = stack.back(); stack.pop_back();
    stack.push_back(std::isalpha(c) || std::isdigit(c) || c == '+' || c == '/' || c == '=');
  }

  void isValidStringChar() {
    int c = stack.back(); stack.pop_back();
    stack.push_back(c >= 0x20 && c <= 0x7E);
  }

  void isValidIdentifier() {
    std::string s = getString();
    if (s.size() == 0 || !isLcAlpha(s[0])) {
      stack.push_back(false);
      return;
    }

    for (char c : s) {
      if (!isValidIdentifierChar(c)) {
        stack.push_back(false);
        return;
      }
    }
    stack.push_back(true);
  }

  void isValidString() {
    std::string s = getString();
    for (char c : s) {
      if (!isValidStringChar(c)) {
        stack.push_back(false);
        return;
      }
    }
    stack.push_back(true);
  }

  void isValidEncodedBinaryContent() {
    std::string s = getString();
    if (s.size() % 4 != 0) {
      stack.push_back(false);
      return;
    }

    bool equalSeen = false;
    for (auto it = s.begin(); it != s.end(); it++) {
      if (*it == '=') {
        equalSeen = true;
      } else if (equalSeen || !isValidEncodedBinaryContentChar(*it)) {
        stack.push_back(false);
        return;
      }
    }
    stack.push_back(true);
  }

  void itemTypeMatchesContent() {
    // This function is deliberately left as an exercise to the reader
    // to convert to VM instructions as it involves complex type checking
    stack.push_back(true);
  }

  void decodeBase64() {
    std::string encoded = getString();
    if (encoded.size() == 0) {
      stack.push_back(false);
      return;
    }

    int padding = 0;
    for (auto it = encoded.rbegin(); padding < 2 && it != encoded.rend() && *it == '='; ++it) {
      ++padding;
    }

    std::string result = Base64::decode(encoded, padding);
    storeString(result);
  }

  void encodeBase64() {
    std::string input = getString();
    std::string result = Base64::encode(folly::ByteRange(reinterpret_cast<const uint8_t*>(input.c_str()), input.length()));
    storeString(result);
  }

  std::string getString() {
    std::string result;
    while (!stack.empty()) {
      result.push_back(static_cast<char>(stack.back()));
      stack.pop_back();
    }
    return result;
  }

  void storeString(const std::string& s) {
    for (char c : s) {
      stack.push_back(c);
    }
  }
};

}
}