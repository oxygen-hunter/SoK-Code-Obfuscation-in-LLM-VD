#include "hermes/VM/detail/IdentifierHashTable.h"

#include "hermes/VM/StringPrimitive.h"

using namespace hermes::vm::detail;
using hermes::vm::StringPrimitive;
using hermes::vm::SymbolID;

enum Instruction {
  PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
};

class VM {
public:
  VM() : pc(0) {}

  void run(const std::vector<int>& code) {
    while (pc < code.size()) {
      int instr = code[pc++];
      switch (instr) {
        case PUSH: stack.push_back(code[pc++]); break;
        case POP: stack.pop_back(); break;
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
        case JMP: pc = code[pc]; break;
        case JZ: {
          int offset = code[pc++];
          if (stack.back() == 0) pc += offset;
          break;
        }
        case LOAD: stack.push_back(memory[code[pc++]]); break;
        case STORE: memory[code[pc++]] = stack.back(); stack.pop_back(); break;
        case CALL: {
          int addr = code[pc++];
          callStack.push_back(pc);
          pc = addr;
          break;
        }
        case RET: {
          pc = callStack.back();
          callStack.pop_back();
          break;
        }
      }
    }
  }

  int top() const {
    return stack.back();
  }

private:
  std::vector<int> stack;
  std::vector<int> callStack;
  std::vector<int> memory = std::vector<int>(256, 0);
  size_t pc;
};

template <typename T>
uint32_t IdentifierHashTable::lookupString(
    llvh::ArrayRef<T> str,
    uint32_t hash,
    bool mustBeNew) const {
  VM vm;
  std::vector<int> code = {
    PUSH, (int)hash, LOAD, 0, PUSH, 1, // initial setup
    CALL, 10,                         // call lookup loop
    RET,                              // return result
    PUSH, (int)capacity() - 1,        // loop start
    JZ, 14,                           // if table is empty
    LOAD, 0,                          // load next index
    PUSH, 1, ADD, STORE, 0,           // advance index
    JMP, -14,                         // jump back to loop start
    LOAD, 0, RET                      // return index
  };
  vm.run(code);
  return vm.top();
}

template uint32_t IdentifierHashTable::lookupString(
    llvh::ArrayRef<char> str,
    uint32_t hash,
    bool mustBeNew) const;

template uint32_t IdentifierHashTable::lookupString(
    llvh::ArrayRef<char16_t> str,
    uint32_t hash,
    bool mustBeNew) const;

uint32_t IdentifierHashTable::lookupString(
    const StringPrimitive *str,
    bool mustBeNew) const {
  if (str->isASCII()) {
    return lookupString(str->castToASCIIRef(), mustBeNew);
  } else {
    return lookupString(str->castToUTF16Ref(), mustBeNew);
  }
}

uint32_t IdentifierHashTable::lookupString(
    const StringPrimitive *str,
    uint32_t hash,
    bool mustBeNew) const {
  if (str->isASCII()) {
    return lookupString(str->castToASCIIRef(), hash, mustBeNew);
  } else {
    return lookupString(str->castToUTF16Ref(), hash, mustBeNew);
  }
}

void IdentifierHashTable::insert(uint32_t idx, SymbolID id) {
  table_.set(idx, id.unsafeGetIndex());
  ++size_;
  ++nonEmptyEntryCount_;

  if (shouldGrow()) {
    growAndRehash(capacity() * 2);
  }
}

void IdentifierHashTable::remove(const StringPrimitive *str) {
  if (str->isASCII()) {
    remove(str->castToASCIIRef());
  } else {
    remove(str->castToUTF16Ref());
  }
}

void IdentifierHashTable::growAndRehash(uint32_t newCapacity) {
  if (LLVM_UNLIKELY(newCapacity <= capacity())) {
    hermes_fatal("too many identifiers created");
  }
  assert(llvh::isPowerOf2_32(newCapacity) && "capacity must be power of 2");
  CompactTable tmpTable(newCapacity, table_.getCurrentScale());
  tmpTable.swap(table_);
  for (uint32_t oldIdx = 0; oldIdx < tmpTable.size(); ++oldIdx) {
    if (!tmpTable.isValid(oldIdx)) {
      continue;
    }
    uint32_t idx = 0;
    uint32_t oldVal = tmpTable.get(oldIdx);
    auto &lookupTableEntry = identifierTable_->getLookupTableEntry(oldVal);
    uint32_t hash = lookupTableEntry.getHash();
    if (lookupTableEntry.isStringPrim()) {
      idx = lookupString(lookupTableEntry.getStringPrim(), hash, true);
    } else if (lookupTableEntry.isLazyASCII()) {
      idx = lookupString(lookupTableEntry.getLazyASCIIRef(), hash, true);
    } else if (lookupTableEntry.isLazyUTF16()) {
      idx = lookupString(lookupTableEntry.getLazyUTF16Ref(), hash, true);
    }
    table_.set(idx, oldVal);
  }
  nonEmptyEntryCount_ = size_;
}