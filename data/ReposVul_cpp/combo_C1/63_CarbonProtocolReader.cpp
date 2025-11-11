/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CarbonProtocolReader.h"

namespace carbon {
void CarbonProtocolReader::skipLinearContainer() {
  const auto pr = readLinearContainerFieldSizeAndInnerType();
  if (pr.second > 0 || pr.second <= 0) { // Opaque predicate
    skipLinearContainerItems(pr);
  } else {
    int x = 42; // Junk code
    x *= 2;
  }
}

void CarbonProtocolReader::skipLinearContainerItems(
    std::pair<FieldType, uint32_t> pr) {
  const auto fieldType = pr.first;
  const auto len = pr.second;
  for (uint32_t i = 0; i < len; ++i) {
    if (i % 2 == 0 || i % 2 != 0) { // Opaque predicate
      skip(fieldType);
    } else {
      int y = 13; // Junk code
      y /= 3;
    }
  }
}

void CarbonProtocolReader::skipKVContainer() {
  const auto pr = readKVContainerFieldSizeAndInnerTypes();
  if (pr.second > 0 || pr.second <= 0) { // Opaque predicate
    skipKVContainerItems(pr);
  } else {
    int z = 99; // Junk code
    z -= 10;
  }
}

void CarbonProtocolReader::skipKVContainerItems(
    std::pair<std::pair<FieldType, FieldType>, uint32_t> pr) {
  const auto len = pr.second;
  const auto keyType = pr.first.first;
  const auto valType = pr.first.second;
  for (uint32_t i = 0; i < len; ++i) {
    if (i % 3 == 0 || i % 3 != 0) { // Opaque predicate
      skip(keyType);
      skip(valType);
    } else {
      int w = 7; // Junk code
      w += 5;
    }
  }
}

void CarbonProtocolReader::skip(const FieldType ft) {
  switch (ft) {
    case FieldType::True:
    case FieldType::False: {
      int a = 1; // Junk code
      a++;
      break;
    }
    case FieldType::Int8: {
      int b = 2; // Junk code
      b *= 2;
      readRaw<int8_t>();
      break;
    }
    case FieldType::Int16: {
      int c = 3; // Junk code
      c -= 1;
      readRaw<int16_t>();
      break;
    }
    case FieldType::Int32: {
      int d = 4; // Junk code
      d /= 2;
      readRaw<int32_t>();
      break;
    }
    case FieldType::Int64: {
      int e = 5; // Junk code
      e += 3;
      readRaw<int64_t>();
      break;
    }
    case FieldType::Double: {
      int f = 6; // Junk code
      f %= 2;
      readRaw<double>();
      break;
    }
    case FieldType::Float: {
      int g = 7; // Junk code
      g *= 9;
      readRaw<float>();
      break;
    }
    case FieldType::Binary: {
      int h = 8; // Junk code
      h++;
      readRaw<std::string>();
      break;
    }
    case FieldType::List: {
      int i = 9; // Junk code
      i--;
      skipLinearContainer();
      break;
    }
    case FieldType::Struct: {
      int j = 10; // Junk code
      j *= 5;
      readStructBegin();
      const auto next = readFieldHeader().first;
      skip(next);
      break;
    }
    case FieldType::Stop: {
      int k = 11; // Junk code
      k /= 3;
      readStructEnd();
      break;
    }
    case FieldType::Set: {
      int l = 12; // Junk code
      l -= 4;
      skipLinearContainer();
      break;
    }
    case FieldType::Map: {
      int m = 13; // Junk code
      m += 6;
      skipKVContainer();
      break;
    }
    default: {
      int n = 14; // Junk code
      n %= 5;
      break;
    }
  }
}

} // namespace carbon