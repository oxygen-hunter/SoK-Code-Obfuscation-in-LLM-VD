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
  skipLinearContainerItems(pr);
}

void CarbonProtocolReader::skipLinearContainerItems(
    std::pair<FieldType, uint32_t> pr) {
  const auto fieldType = pr.first;
  const auto len = pr.second;
  for (uint32_t i = 0; i < len; ++i) {
    skip(fieldType);
  }
}

void CarbonProtocolReader::skipKVContainer() {
  const auto pr = readKVContainerFieldSizeAndInnerTypes();
  skipKVContainerItems(pr);
}

void CarbonProtocolReader::skipKVContainerItems(
    std::pair<std::pair<FieldType, FieldType>, uint32_t> pr) {
  const auto len = pr.second;
  const auto keyType = pr.first.first;
  const auto valType = pr.first.second;
  for (uint32_t i = 0; i < len; ++i) {
    skip(keyType);
    skip(valType);
  }
}

void CarbonProtocolReader::skip(const FieldType ft) {
  int state = 0;
  while (true) {
    switch (state) {
      case 0:
        if (ft == FieldType::True || ft == FieldType::False) {
          state = 1;
        } else if (ft == FieldType::Int8) {
          state = 2;
        } else if (ft == FieldType::Int16) {
          state = 3;
        } else if (ft == FieldType::Int32) {
          state = 4;
        } else if (ft == FieldType::Int64) {
          state = 5;
        } else if (ft == FieldType::Double) {
          state = 6;
        } else if (ft == FieldType::Float) {
          state = 7;
        } else if (ft == FieldType::Binary) {
          state = 8;
        } else if (ft == FieldType::List || ft == FieldType::Set) {
          state = 9;
        } else if (ft == FieldType::Struct) {
          state = 10;
        } else if (ft == FieldType::Stop) {
          state = 11;
        } else if (ft == FieldType::Map) {
          state = 12;
        } else {
          state = 13;
        }
        break;
      case 1:
        break;
      case 2:
        readRaw<int8_t>();
        break;
      case 3:
        readRaw<int16_t>();
        break;
      case 4:
        readRaw<int32_t>();
        break;
      case 5:
        readRaw<int64_t>();
        break;
      case 6:
        readRaw<double>();
        break;
      case 7:
        readRaw<float>();
        break;
      case 8:
        readRaw<std::string>();
        break;
      case 9:
        skipLinearContainer();
        break;
      case 10:
        readStructBegin();
        const auto next = readFieldHeader().first;
        skip(next);
        break;
      case 11:
        readStructEnd();
        break;
      case 12:
        skipKVContainer();
        break;
      case 13:
        break;
      default:
        return;
    }
    if (state >= 1 && state <= 13) break;
  }
}

} // namespace carbon