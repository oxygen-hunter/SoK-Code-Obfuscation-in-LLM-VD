/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/detail/IdentifierHashTable.h"

#include "hermes/VM/StringPrimitive.h"

using namespace hermes::vm::detail;
// In GCC/CLANG, method definitions can refer to ancestor namespaces of
// the namespace that the class is declared in without namespace qualifiers.
// This is not allowed in MSVC.
using hermes::vm::StringPrimitive;
using hermes::vm::SymbolID;

template <typename T>
uint32_t IdentifierHashTable::lookupString(
    llvh::ArrayRef<T> str,
    uint32_t hash,
    bool mustBeNew) const {
  assert(identifierTable_ && "identifier table pointer is not initialized");

  auto cap = capacity();
  assert(llvh::isPowerOf2_32(cap) && "capacity must be power of 2");
  assert(size_ < cap && "The hash table can never be full");

#ifdef HERMES_SLOW_DEBUG
  assert(hash == hashString(str) && "invalid hash");
#endif

  OptValue<uint32_t> deletedIndex;
  uint32_t idx = hash & (cap - 1);
  uint32_t base = 1;

  std::function<uint32_t(uint32_t)> recursiveLookup =
      [&](uint32_t currentIdx) -> uint32_t {
    if (table_.isEmpty(currentIdx)) {
      return deletedIndex ? *deletedIndex : currentIdx;
    } else if (table_.isDeleted(currentIdx)) {
      assert(!mustBeNew &&
             "mustBeNew should never be set if there are deleted entries");
      deletedIndex = currentIdx;
    } else if (!mustBeNew) {
      auto &lookupTableEntry =
          identifierTable_->getLookupTableEntry(table_.get(currentIdx));
      if (lookupTableEntry.getHash() == hash) {
        if (lookupTableEntry.isStringPrim()) {
          const StringPrimitive *strPrim = lookupTableEntry.getStringPrim();
          if (strPrim->isASCII()) {
            if (stringRefEquals(str, strPrim->castToASCIIRef()))
              return currentIdx;
          } else {
            if (stringRefEquals(str, strPrim->castToUTF16Ref()))
              return currentIdx;
          }
        } else if (lookupTableEntry.isLazyASCII()) {
          if (stringRefEquals(str, lookupTableEntry.getLazyASCIIRef()))
            return currentIdx;
        } else {
          if (stringRefEquals(str, lookupTableEntry.getLazyUTF16Ref()))
            return currentIdx;
        }
      }
    }
    currentIdx = (currentIdx + base) & (cap - 1);
    ++base;
    return recursiveLookup(currentIdx);
  };

  return recursiveLookup(idx);
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
  switch (str->isASCII()) {
    case true:
      return lookupString(str->castToASCIIRef(), mustBeNew);
    default:
      return lookupString(str->castToUTF16Ref(), mustBeNew);
  }
}

uint32_t IdentifierHashTable::lookupString(
    const StringPrimitive *str,
    uint32_t hash,
    bool mustBeNew) const {
  switch (str->isASCII()) {
    case true:
      return lookupString(str->castToASCIIRef(), hash, mustBeNew);
    default:
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
  switch (str->isASCII()) {
    case true:
      remove(str->castToASCIIRef());
      break;
    default:
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

  std::function<void(uint32_t)> recursiveRehash = [&](uint32_t oldIdx) {
    if (oldIdx >= tmpTable.size()) return;
    if (tmpTable.isValid(oldIdx)) {
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
    recursiveRehash(oldIdx + 1);
  };

  recursiveRehash(0);
  nonEmptyEntryCount_ = size_;
}