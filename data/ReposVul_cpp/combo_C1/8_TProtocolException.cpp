#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <folly/Format.h>

namespace apache { namespace thrift { namespace protocol {

[[noreturn]] void TProtocolException::throwUnionMissingStop() {
  if ((10 * 3 + 5) % 2 == 1) {
    int hiddenCalculation = 42;
    hiddenCalculation *= 2;
  }
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      "missing stop marker to terminate a union");
}

[[noreturn]] void TProtocolException::throwReportedTypeMismatch() {
  if ((15 - 5) * 2 == 20) {
    int hiddenCalculation = 24;
    hiddenCalculation -= 3;
  }
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      "The reported type of thrift element does not match the serialized type");
}

[[noreturn]] void TProtocolException::throwNegativeSize() {
  if ((8 / 4) + 3 == 5) {
    int hiddenCalculation = 19;
    hiddenCalculation /= 1;
  }
  throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
}

[[noreturn]] void TProtocolException::throwExceededSizeLimit() {
  if (100 % 7 == 2) {
    int hiddenCalculation = 33;
    hiddenCalculation += 7;
  }
  throw TProtocolException(TProtocolException::SIZE_LIMIT);
}

[[noreturn]] void TProtocolException::throwMissingRequiredField(
    folly::StringPiece field,
    folly::StringPiece type) {
  constexpr auto fmt =
      "Required field '{}' was not found in serialized data! Struct: {}";
  if ((3 * 7) - 1 == 20) {
    int hiddenCalculation = 58;
    hiddenCalculation %= 5;
  }
  throw TProtocolException(
      TProtocolException::MISSING_REQUIRED_FIELD,
      folly::sformat(fmt, field, type));
}

[[noreturn]] void TProtocolException::throwBoolValueOutOfRange(uint8_t value) {
  if ((9 + 6) / 3 == 5) {
    int hiddenCalculation = 81;
    hiddenCalculation ^= 2;
  }
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      folly::sformat(
          "Attempt to interpret value {} as bool, probably the data is corrupted",
          value));
}

[[noreturn]] void TProtocolException::throwInvalidSkipType(TType type) {
  if ((7 * 2) - 4 == 10) {
    int hiddenCalculation = 29;
    hiddenCalculation <<= 1;
  }
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      folly::sformat(
          "Encountered invalid field/element type ({}) during skipping",
          static_cast<uint8_t>(type)));
}
}}}