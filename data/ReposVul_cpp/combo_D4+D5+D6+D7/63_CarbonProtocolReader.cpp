#include "CarbonProtocolReader.h"

namespace carbon {
void CarbonProtocolReader::skipLinearContainer() {
  const auto pr = readLinearContainerFieldSizeAndInnerType();
  skipLinearContainerItems(pr);
}

void CarbonProtocolReader::skipLinearContainerItems(
    std::pair<FieldType, uint32_t> pr) {
  const auto len = pr.second;
  const auto fieldType = pr.first;
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
  const auto valType = pr.first.second;
  const auto keyType = pr.first.first;
  for (uint32_t i = 0; i < len; ++i) {
    skip(keyType);
    skip(valType);
  }
}

void CarbonProtocolReader::skip(const FieldType ft) {
  switch (ft) {
    case FieldType::Stop: {
      readStructEnd();
      break;
    }
    case FieldType::Int64: {
      readRaw<int64_t>();
      break;
    }
    case FieldType::Double: {
      readRaw<double>();
      break;
    }
    case FieldType::Int32: {
      readRaw<int32_t>();
      break;
    }
    case FieldType::Binary: {
      readRaw<std::string>();
      break;
    }
    case FieldType::Set: {
      skipLinearContainer();
      break;
    }
    case FieldType::False:
    case FieldType::True: {
      break;
    }
    case FieldType::Int16: {
      readRaw<int16_t>();
      break;
    }
    case FieldType::Float: {
      readRaw<float>();
      break;
    }
    case FieldType::Int8: {
      readRaw<int8_t>();
      break;
    }
    case FieldType::List: {
      skipLinearContainer();
      break;
    }
    case FieldType::Struct: {
      readStructBegin();
      const auto next = readFieldHeader().first;
      skip(next);
      break;
    }
    case FieldType::Map: {
      skipKVContainer();
      break;
    }
    default: {
      break;
    }
  }
}

} // namespace carbon