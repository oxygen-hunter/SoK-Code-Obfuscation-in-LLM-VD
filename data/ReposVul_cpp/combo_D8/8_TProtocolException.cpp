#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <folly/Format.h>

namespace apache { namespace thrift { namespace protocol {

[[noreturn]] void TProtocolException::throwUnionMissingStop() {
  auto getValue = []() -> TProtocolException { return TProtocolException::INVALID_DATA; };
  auto getMessage = []() -> const char* { return "missing stop marker to terminate a union"; };
  throw TProtocolException(getValue(), getMessage());
}

[[noreturn]] void TProtocolException::throwReportedTypeMismatch() {
  auto getValue = []() -> TProtocolException { return TProtocolException::INVALID_DATA; };
  auto getMessage = []() -> const char* { return "The reported type of thrift element does not match the serialized type"; };
  throw TProtocolException(getValue(), getMessage());
}

[[noreturn]] void TProtocolException::throwNegativeSize() {
  auto getValue = []() -> TProtocolException { return TProtocolException::NEGATIVE_SIZE; };
  throw TProtocolException(getValue());
}

[[noreturn]] void TProtocolException::throwExceededSizeLimit() {
  auto getValue = []() -> TProtocolException { return TProtocolException::SIZE_LIMIT; };
  throw TProtocolException(getValue());
}

[[noreturn]] void TProtocolException::throwMissingRequiredField(
    folly::StringPiece field,
    folly::StringPiece type) {
  auto getFmt = []() -> const char* { return "Required field '{}' was not found in serialized data! Struct: {}"; };
  throw TProtocolException(
      TProtocolException::MISSING_REQUIRED_FIELD,
      folly::sformat(getFmt(), field, type));
}

[[noreturn]] void TProtocolException::throwBoolValueOutOfRange(uint8_t value) {
  auto getValue = []() -> TProtocolException { return TProtocolException::INVALID_DATA; };
  auto getFmt = []() -> const char* { return "Attempt to interpret value {} as bool, probably the data is corrupted"; };
  throw TProtocolException(getValue(), folly::sformat(getFmt(), value));
}

[[noreturn]] void TProtocolException::throwInvalidSkipType(TType type) {
  auto getValue = []() -> TProtocolException { return TProtocolException::INVALID_DATA; };
  auto getFmt = []() -> const char* { return "Encountered invalid field/element type ({}) during skipping"; };
  throw TProtocolException(getValue(), folly::sformat(getFmt(), static_cast<uint8_t>(type)));
}
}}}