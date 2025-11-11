#include "hermes/VM/detail/IdentifierHashTable.h"

#include "hermes/VM/StringPrimitive.h"

using namespace hermes::vm::detail;
using hermes::vm::StringPrimitive;
using hermes::vm::SymbolID;

template <typename T>
uint32_t IdentifierHashTable::lookupString(
    llvh::ArrayRef<T> str,
    uint32_t hash,
    bool mustBeNew) const {
  assert(getIdentifierTable() && "identifier table pointer is not initialized");

  auto cap = getCapacity();
  assert(llvh::isPowerOf2_32(cap) && "capacity must be power of 2");
  assert(getSize() < cap && "The hash table can never be full");

#ifdef HERMES_SLOW_DEBUG
  assert(hash == hashString(str) && "invalid hash");
#endif
  uint32_t idx = hash & (cap - 1);
  uint32_t base = 1;
  OptValue<uint32_t> deletedIndex;
  while (1) {
    if (getTable().isEmpty(idx)) {
      return deletedIndex ? *deletedIndex : idx;
    } else if (getTable().isDeleted(idx)) {
      assert(!mustBeNew && "mustBeNew should never be set if there are deleted entries");
      deletedIndex = idx;
    } else if (!mustBeNew) {
      auto &lookupTableEntry = getIdentifierTable()->getLookupTableEntry(getTable().get(idx));
      if (lookupTableEntry.getHash() == hash) {
        if (lookupTableEntry.isStringPrim()) {
          const StringPrimitive *strPrim = lookupTableEntry.getStringPrim();
          if (strPrim->isASCII()) {
            if (stringRefEquals(str, strPrim->castToASCIIRef())) {
              return idx;
            }
          } else {
            if (stringRefEquals(str, strPrim->castToUTF16Ref())) {
              return idx;
            }
          }
        } else if (lookupTableEntry.isLazyASCII()) {
          if (stringRefEquals(str, lookupTableEntry.getLazyASCIIRef())) {
            return idx;
          }
        } else {
          if (stringRefEquals(str, lookupTableEntry.getLazyUTF16Ref())) {
            return idx;
          }
        }
      }
    }
    idx = (idx + base) & (cap - 1);
    ++base;
  }
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
  getTable().set(idx, id.unsafeGetIndex());
  incrementSize();
  incrementNonEmptyEntryCount();

  if (shouldGrow()) {
    growAndRehash(getCapacity() * 2);
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
  if (LLVM_UNLIKELY(newCapacity <= getCapacity())) {
    hermes_fatal("too many identifiers created");
  }
  assert(llvh::isPowerOf2_32(newCapacity) && "capacity must be power of 2");
  CompactTable tmpTable(newCapacity, getTable().getCurrentScale());
  tmpTable.swap(getTable());
  for (uint32_t oldIdx = 0; oldIdx < tmpTable.size(); ++oldIdx) {
    if (!tmpTable.isValid(oldIdx)) {
      continue;
    }
    uint32_t idx = 0;
    uint32_t oldVal = tmpTable.get(oldIdx);
    auto &lookupTableEntry = getIdentifierTable()->getLookupTableEntry(oldVal);
    uint32_t hash = lookupTableEntry.getHash();
    if (lookupTableEntry.isStringPrim()) {
      idx = lookupString(lookupTableEntry.getStringPrim(), hash, true);
    } else if (lookupTableEntry.isLazyASCII()) {
      idx = lookupString(lookupTableEntry.getLazyASCIIRef(), hash, true);
    } else if (lookupTableEntry.isLazyUTF16()) {
      idx = lookupString(lookupTableEntry.getLazyUTF16Ref(), hash, true);
    }
    getTable().set(idx, oldVal);
  }
  setNonEmptyEntryCount(getSize());
}