#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cassert>
#include <cstdlib>

namespace HPHP {

enum Instruction {
  PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
};

class VirtualMachine {
public:
  VirtualMachine(const std::vector<int>& instructions)
    : m_instructions(instructions), m_ip(0), m_stack(), m_memory(1024, 0) {}

  void run() {
    bool running = true;
    while (running) {
      switch (m_instructions[m_ip]) {
        case PUSH:
          m_stack.push_back(m_instructions[++m_ip]);
          break;
        case POP:
          m_stack.pop_back();
          break;
        case ADD: {
          int b = m_stack.back(); m_stack.pop_back();
          int a = m_stack.back(); m_stack.pop_back();
          m_stack.push_back(a + b);
          break;
        }
        case SUB: {
          int b = m_stack.back(); m_stack.pop_back();
          int a = m_stack.back(); m_stack.pop_back();
          m_stack.push_back(a - b);
          break;
        }
        case JMP:
          m_ip = m_instructions[++m_ip] - 1;
          break;
        case JZ: {
          int target = m_instructions[++m_ip];
          if (m_stack.back() == 0) {
            m_ip = target - 1;
          }
          break;
        }
        case LOAD:
          m_stack.push_back(m_memory[m_instructions[++m_ip]]);
          break;
        case STORE:
          m_memory[m_instructions[++m_ip]] = m_stack.back();
          m_stack.pop_back();
          break;
        case CALL:
          m_stack.push_back(m_ip + 2);
          m_ip = m_instructions[++m_ip] - 1;
          break;
        case RET:
          m_ip = m_stack.back() - 1;
          m_stack.pop_back();
          break;
        case HALT:
          running = false;
          break;
        default:
          std::cerr << "Unknown instruction: " << m_instructions[m_ip] << "\n";
          running = false;
          break;
      }
      ++m_ip;
    }
  }

  int getMemory(int address) const {
    return m_memory[address];
  }

private:
  std::vector<int> m_instructions;
  int m_ip;
  std::vector<int> m_stack;
  std::vector<int> m_memory;
};

struct MemFile {
  MemFile(const std::string& wrapper, const std::string& stream)
    : m_data(nullptr), m_len(-1), m_cursor(0), m_malloced(false) {
    initialize();
  }

  MemFile(const char *data, int64_t len, const std::string& wrapper, const std::string& stream)
    : m_data(nullptr), m_len(len), m_cursor(0), m_malloced(true) {
    m_data = static_cast<char*>(malloc(len + 1));
    if (m_data && len) {
      memcpy(m_data, data, len);
    }
    m_data[len] = '\0';
    initialize();
  }

  ~MemFile() {
    close();
  }

  bool open(const std::string& filename, const std::string& mode) {
    // Obfuscated open logic
    std::vector<int> instructions = {
      PUSH, -1, LOAD, 0, JZ, 15,
      LOAD, 1, JZ, 18, LOAD, 2, JZ, 21,
      PUSH, 1, STORE, 3, HALT,
      PUSH, 0, STORE, 3, HALT,
      PUSH, 2, STORE, 3, HALT
    };
    VirtualMachine vm(instructions);
    vm.run();
    m_len = vm.getMemory(3);
    return m_len != -1;
  }

  bool close() {
    if (m_malloced && m_data) {
      free(m_data);
      m_data = nullptr;
    }
    return true;
  }

private:
  char* m_data;
  int64_t m_len;
  int64_t m_cursor;
  bool m_malloced;

  void initialize() {
    // Obfuscated initialization logic
    std::vector<int> instructions = {
      PUSH, 0, STORE, 0, HALT
    };
    VirtualMachine vm(instructions);
    vm.run();
  }
};

}