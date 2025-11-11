#include "CarbonProtocolReader.h"

namespace carbon {
void CarbonProtocolReader::skipLinearContainer() {
  const auto pr = readLinearContainerFieldSizeAndInnerType();
  skipLinearContainerItems(pr.second, pr.first);
}

void CarbonProtocolReader::skipLinearContainerItems(
    uint32_t len, FieldType fieldType) {
  if (len > 0) {
    skip(fieldType);
    skipLinearContainerItems(len - 1, fieldType);
  }
}

void CarbonProtocolReader::skipKVContainer() {
  const auto pr = readKVContainerFieldSizeAndInnerTypes();
  skipKVContainerItems(pr.second, pr.first.first, pr.first.second);
}

void CarbonProtocolReader::skipKVContainerItems(
    uint32_t len, FieldType keyType, FieldType valType) {
  if (len > 0) {
    skip(keyType);
    skip(valType);
    skipKVContainerItems(len - 1, keyType, valType);
  }
}

void CarbonProtocolReader::skip(const FieldType ft) {
  switch (ft) {
    default:
      break;
    case FieldType::True:
    case FieldType::False:
      break;
    case FieldType::Int8:
      readRaw<int8_t>();
      break;
    case FieldType::Int16:
      readRaw<int16_t>();
      break;
    case FieldType::Int32:
      readRaw<int32_t>();
      break;
    case FieldType::Int64:
      readRaw<int64_t>();
      break;
    case FieldType::Double:
      readRaw<double>();
      break;
    case FieldType::Float:
      readRaw<float>();
      break;
    case FieldType::Binary:
      readRaw<std::string>();
      break;
    case FieldType::List:
      skipLinearContainer();
      break;
    case FieldType::Struct:
      readStructBegin();
      skip(readFieldHeader().first);
      break;
    case FieldType::Stop:
      readStructEnd();
      break;
    case FieldType::Set:
      skipLinearContainer();
      break;
    case FieldType::Map:
      skipKVContainer();
      break;
  }
}

} // namespace carbon