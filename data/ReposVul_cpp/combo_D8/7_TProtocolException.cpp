#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <folly/Format.h>

namespace apache { namespace thrift { namespace protocol {

namespace {
  int getInvalidData() { return TProtocolException::INVALID_DATA; }
  int getNegativeSize() { return TProtocolException::NEGATIVE_SIZE; }
  int getSizeLimit() { return TProtocolException::SIZE_LIMIT; }
  int getMissingRequiredField() { return TProtocolException::MISSING_REQUIRED_FIELD; }
  constexpr const char* getFormatString() {
    return "Required field '{}' was not found in serialized data! Struct: {}";
  }
}

[[noreturn]] void TProtocolException::throwUnionMissingStop() {
  throw TProtocolException(
      getInvalidData(),
      "missing stop marker to terminate a union");
}

[[noreturn]] void TProtocolException::throwReportedTypeMismatch() {
  throw TProtocolException(
      getInvalidData(),
      "The reported type of thrift element does not match the serialized type");
}

[[noreturn]] void TProtocolException::throwNegativeSize() {
  throw TProtocolException(getNegativeSize());
}

[[noreturn]] void TProtocolException::throwExceededSizeLimit() {
  throw TProtocolException(getSizeLimit());
}

[[noreturn]] void TProtocolException::throwMissingRequiredField(
    folly::StringPiece field,
    folly::StringPiece type) {
  throw TProtocolException(
      getMissingRequiredField(),
      folly::sformat(getFormatString(), field, type));
}

[[noreturn]] void TProtocolException::throwBoolValueOutOfRange(uint8_t value) {
  throw TProtocolException(
      getInvalidData(),
      folly::sformat(
          "Attempt to interpret value {} as bool, probably the data is corrupted",
          value));
}

[[noreturn]] void TProtocolException::throwInvalidSkipType(TType type) {
  throw TProtocolException(
      getInvalidData(),
      folly::sformat(
          "Encountered invalid field/element type ({}) during skipping",
          static_cast<uint8_t>(type)));
}
}}}