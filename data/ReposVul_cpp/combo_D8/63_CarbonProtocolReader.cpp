#include "CarbonProtocolReader.h"

namespace carbon {
void CarbonProtocolReader::skipLinearContainer() {
  const auto pr = readLinearContainerFieldSizeAndInnerType();
  skipLinearContainerItems(pr);
}

void CarbonProtocolReader::skipLinearContainerItems(
    std::pair<FieldType, uint32_t> pr) {
  const auto getFieldType = [&]() { return pr.first; };
  const auto getLen = [&]() { return pr.second; };
  for (uint32_t i = 0; i < getLen(); ++i) {
    skip(getFieldType());
  }
}

void CarbonProtocolReader::skipKVContainer() {
  const auto pr = readKVContainerFieldSizeAndInnerTypes();
  skipKVContainerItems(pr);
}

void CarbonProtocolReader::skipKVContainerItems(
    std::pair<std::pair<FieldType, FieldType>, uint32_t> pr) {
  const auto getLen = [&]() { return pr.second; };
  const auto getKeyType = [&]() { return pr.first.first; };
  const auto getValType = [&]() { return pr.first.second; };
  for (uint32_t i = 0; i < getLen(); ++i) {
    skip(getKeyType());
    skip(getValType());
  }
}

void CarbonProtocolReader::skip(const FieldType ft) {
  switch (ft) {
    case FieldType::True:
    case FieldType::False: {
      break;
    }
    case FieldType::Int8: {
      readRaw<int8_t>();
      break;
    }
    case FieldType::Int16: {
      readRaw<int16_t>();
      break;
    }
    case FieldType::Int32: {
      readRaw<int32_t>();
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
    case FieldType::Float: {
      readRaw<float>();
      break;
    }
    case FieldType::Binary: {
      readRaw<std::string>();
      break;
    }
    case FieldType::List: {
      skipLinearContainer();
      break;
    }
    case FieldType::Struct: {
      readStructBegin();
      const auto getNext = [&]() { return readFieldHeader().first; };
      skip(getNext());
      break;
    }
    case FieldType::Stop: {
      readStructEnd();
      break;
    }
    case FieldType::Set: {
      skipLinearContainer();
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