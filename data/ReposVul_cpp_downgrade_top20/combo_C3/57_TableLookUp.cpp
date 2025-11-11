#include "common/TableLookUp.h"
#include "decoders/RawDecoderException.h"
#include <cassert>
#include <vector>

namespace rawspeed {

typedef unsigned short ushort16;

class VM {
public:
  VM() : pc(0) {}

  enum Instructions {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT
  };

  void execute(const std::vector<int>& bytecode, std::vector<ushort16>& memory, int ntables, bool dither) {
    pc = 0;
    stack.clear();
    this->memory = &memory;
    this->ntables = ntables;
    this->dither = dither;

    while (pc < bytecode.size()) {
      int instr = bytecode[pc++];
      switch(instr) {
        case PUSH: {
          stack.push_back(bytecode[pc++]);
          break;
        }
        case POP: {
          stack.pop_back();
          break;
        }
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
        case JMP: {
          pc = bytecode[pc];
          break;
        }
        case JZ: {
          int addr = bytecode[pc++];
          if (stack.back() == 0) pc = addr;
          stack.pop_back();
          break;
        }
        case LOAD: {
          int addr = stack.back();
          stack.pop_back();
          stack.push_back((*memory)[addr]);
          break;
        }
        case STORE: {
          int value = stack.back();
          stack.pop_back();
          int addr = stack.back();
          stack.pop_back();
          (*memory)[addr] = value;
          break;
        }
        case HALT: {
          return;
        }
      }
    }
  }

private:
  int pc;
  std::vector<int> stack;
  std::vector<ushort16>* memory;
  int ntables;
  bool dither;
};

const int TABLE_SIZE = 65536 * 2;

class TableLookUp {
public:
  TableLookUp(int _ntables, bool _dither)
      : ntables(_ntables), dither(_dither) {
    if (ntables < 1) {
      ThrowRDE("Cannot construct 0 tables");
    }
    tables.resize(ntables * TABLE_SIZE, ushort16(0));
    vm = new VM();
  }

  void setTable(int ntable, const std::vector<ushort16>& table) {
    std::vector<int> bytecode = compileSetTable(ntable, table);
    vm->execute(bytecode, tables, ntables, dither);
  }

  ushort16* getTable(int n) {
    if (n > ntables) {
      ThrowRDE("Table lookup with number greater than number of tables.");
    }
    return &tables[n * TABLE_SIZE];
  }

private:
  int ntables;
  bool dither;
  std::vector<ushort16> tables;
  VM* vm;

  std::vector<int> compileSetTable(int ntable, const std::vector<ushort16>& table) {
    std::vector<int> bytecode;
    bytecode.push_back(VM::PUSH);
    bytecode.push_back(ntable);
    bytecode.push_back(VM::PUSH);
    bytecode.push_back(ntables);
    bytecode.push_back(VM::SUB);
    bytecode.push_back(VM::JZ);
    bytecode.push_back(100);
    bytecode.push_back(VM::PUSH);
    bytecode.push_back(table.size());
    bytecode.push_back(VM::PUSH);
    bytecode.push_back(65536);
    bytecode.push_back(VM::SUB);
    bytecode.push_back(VM::JZ);
    bytecode.push_back(110);
    bytecode.push_back(VM::LOAD);
    bytecode.push_back(ntable * TABLE_SIZE);
    bytecode.push_back(VM::PUSH);
    bytecode.push_back(dither ? 1 : 0);
    bytecode.push_back(VM::STORE);
    bytecode.push_back(VM::HALT);
    bytecode.push_back(100); // ThrowRDE for ntables
    bytecode.push_back(VM::HALT);
    bytecode.push_back(110); // ThrowRDE for table size
    bytecode.push_back(VM::HALT);
    return bytecode;
  }
};

} // namespace rawspeed